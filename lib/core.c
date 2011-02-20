#define _GNU_SOURCE

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

// Handle non-standardized systems:
#ifndef WIN32
#include <unistd.h>
#endif

// All curl stuff we need:
#include <curl/multi.h>

// Own headers:
#include "stringop.h"
#include "core.h"

/*--------------------------------------------------------*/

int glyr_message(int v, glyr_settings_t * s, FILE * stream, const char * fmt, ...)
{
    int r=0;
    if(s || v == -1)
    {
        if(v == -1 || v <= s->verbosity)
        {
            va_list param;
            char * tmp = NULL;
            if(stream && fmt)
            {
                va_start(param,fmt);
                r = vasprintf (&tmp, fmt, param);
                va_end(param);

                if(r != -1 && tmp)
                {
                    r = fprintf(stream,"%s%s",(v>2)?C_B"DEBUG: "C_:"",tmp);
                    free(tmp);
                    tmp = NULL;
                }
            }
        }
    }
    return r;
}

/*--------------------------------------------------------*/

// Sleep usec microseconds
static int DL_usleep(long usec)
{
    struct timespec req= { 0 };
    time_t sec = (int) (usec / 1e6L);

    usec = usec - (sec*1e6L);
    req.tv_sec  = (sec);
    req.tv_nsec = (usec*1e3L);

    while(nanosleep(&req , &req) == -1)
    {
        continue;
    }

    return 0;
}

/*--------------------------------------------------------*/

memCache_t * DL_copy(memCache_t * src)
{
	memCache_t * dest = NULL;
	if(src) 
	{
		dest = DL_init();
		if(dest)
		{
			if(src->data)
			{
				dest->data = calloc(src->size+1,sizeof(char));
				memcpy(dest->data,src->data,src->size);
			}
			if(src->dsrc)
			{
				dest->dsrc = strdup(src->dsrc);
			}
			dest->size = src->size;
		}
	}	
	return dest;
}

/*--------------------------------------------------------*/

// cache incoming data in a MemChunk_t
static size_t DL_buffer(void *puffer, size_t size, size_t nmemb, void *cache)
{
    size_t realsize = size * nmemb;
    struct memCache_t *mem = (struct memCache_t *)cache;

    mem->data = realloc(mem->data, mem->size + realsize + 1);
    if (mem->data)
    {
        memcpy(&(mem->data[mem->size]), puffer, realsize);
        mem->size += realsize;
        mem->data[mem->size] = 0;
    }
    else
    {
        glyr_message(-1,NULL,stderr,"Caching failed: Out of memory.\n");
    }
    return realsize;
}

/*--------------------------------------------------------*/

// cleanup internal buffer if no longer used
void DL_free(memCache_t *cache)
{
    if(cache)
    {
        if(cache->size && cache->data)
        {
            free(cache->data);
            cache->data   = NULL;
        }
        if(cache->dsrc)
        {
            free(cache->dsrc);
            cache->dsrc   = NULL;
        }

        cache->size   = 0;

        free(cache);
        cache = NULL;
    }
}

/*--------------------------------------------------------*/

// Use this to init the internal buffer
memCache_t* DL_init(void)
{
    memCache_t *cache = malloc(sizeof(memCache_t));

    if(cache)
    {
        cache->size   = 0;
        cache->data   = NULL;
        cache->dsrc   = NULL;
    }
    else
    {
        glyr_message(-1,NULL,stderr,"Warning: empty dlcache...\n");
    }
    return cache;
}

/*--------------------------------------------------------*/

cache_list * DL_new_lst(void)
{
    cache_list * c = calloc(1,sizeof(cache_list));
    if(c != NULL)
    {
	c->size = 0;
    	c->list = calloc(1,sizeof(memCache_t*));
	if(!c->list)
	{
		glyr_message(-1,NULL,stderr,"calloc() returned NULL!");
	}
    }
    else
    {
	glyr_message(-1,NULL,stderr,"c is still empty?");
    }

    return c;
}

/*--------------------------------------------------------*/

void DL_add_to_list(cache_list * l, memCache_t * c)
{
   if(l && c)
   {
	l->list = realloc(l->list, sizeof(memCache_t *) * (l->size+2));
	l->list[  l->size] = c;
	l->list[++l->size] = NULL;
   } 
}

/*--------------------------------------------------------*/

void DL_push_sublist(cache_list * l, cache_list * s)
{
	if(l && s)
	{
		size_t i;
		for(i = 0; i < s->size; i++)
		{
			DL_add_to_list(l,s->list[i]);
		}
	}
}

/*--------------------------------------------------------*/

// Free list itself and all caches stored in there 
void DL_free_lst(cache_list * c)
{
    if(c)
    {
        size_t i;
        for(i = 0; i < c->size ; i++)
            DL_free(c->list[i]);
        
	DL_free_container(c);
    }
}

/*--------------------------------------------------------*/

// only free list, caches are left intact
void DL_free_container(cache_list * c)
{
	if(c!=NULL)
	{
		if(c->list)
		   free(c->list);

		memset(c,0,sizeof(cache_list));
		free(c);		
	}
}

/*--------------------------------------------------------*/

// Init an easyhandler with all relevant options
static void DL_setopt(CURL *eh, memCache_t * cache, const char * url, glyr_settings_t * s, void * magic_private_ptr, long timeout)
{
    if(!s) return;

    // Set options (see 'man curl_easy_setopt')
    curl_easy_setopt(eh, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(eh, CURLOPT_NOSIGNAL, 1L);

    // last.fm and discogs require an useragent (wokrs without too)
    curl_easy_setopt(eh, CURLOPT_USERAGENT, "Gullible Kunrad the Owl");
    curl_easy_setopt(eh, CURLOPT_HEADER, 0L);

    // Pass vars to curl
    curl_easy_setopt(eh, CURLOPT_URL, url);
    curl_easy_setopt(eh, CURLOPT_PRIVATE, magic_private_ptr);
    curl_easy_setopt(eh, CURLOPT_VERBOSE, (s->verbosity == 4));
    curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, DL_buffer);
    curl_easy_setopt(eh, CURLOPT_WRITEDATA, (void *)cache);

    // amazon plugin requires redirects
    curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(eh, CURLOPT_MAXREDIRS, s->redirects);

    // Discogs equires gzip compression
    curl_easy_setopt(eh, CURLOPT_ENCODING,"gzip");

    // Don't save cookies - I had some quite embarassing moments
    // when amazon's startpage showed me "Hooray for Boobies",
    // because I searched for the Bloodhoundgang album... :(
    // (because I have it already! :-) )
    curl_easy_setopt(eh, CURLOPT_COOKIEJAR ,"");
}

/*--------------------------------------------------------*/

// Init a callback object and a curl_easy_handle
static memCache_t * handle_init(CURLM * cm, cb_object * capo, glyr_settings_t *s, long timeout)
{
    // You did sth. wrong..
    if(!capo || !capo->url)
        return NULL;

    // Init handle
    CURL *eh = curl_easy_init();

    // Init cache
    memCache_t *dlcache = DL_init();

    // Set handle
    capo->handle = eh;

    // Configure curl
    DL_setopt(eh, dlcache, capo->url, s,(void*)capo,timeout);

    // Add handle to multihandle
    curl_multi_add_handle(cm, eh);
    return dlcache;
}

/*--------------------------------------------------------*/

// Download a singe file NOT in parallel
memCache_t * download_single(const char* url, glyr_settings_t * s)
{
    if(url==NULL)
    {
        return NULL;
    }

    //fprintf(stderr,"Downloading <%s>\n",url);

    CURL *curl = NULL;
    CURLcode res = 0;

    if(curl_global_init(CURL_GLOBAL_ALL))
    {
        glyr_message(-1,NULL,stderr,"?? libcurl failed to init.\n");
    }

    // Init handles
    curl = curl_easy_init();
    memCache_t * dldata = DL_init();

    if(curl != NULL)
    {
        // Configure curl
        DL_setopt(curl,dldata,url,s,NULL,s->timeout);

        // Perform transaction
        res = curl_easy_perform(curl);

        // Better check again
        if(res != CURLE_OK)
        {
            glyr_message(-1,NULL,stderr,C_"\n:: %s\n", curl_easy_strerror(res));
            DL_free(dldata);
            dldata = NULL;
        }

        // Handle without any use - clean up
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return dldata;
    }

    // Free mem
    curl_global_cleanup();
    DL_free(dldata);
    return NULL;
}

/*--------------------------------------------------------*/

cache_list * invoke(cb_object *oblist, long CNT, long parallel, long timeout, glyr_settings_t * s)
{
    // curl multi handles
    CURLM *cm;
    CURLMsg *msg;

    // select()
    long L;
    unsigned int C = 0;
    int M, Q, U = -1;
    fd_set R, W, E;
    struct timeval T;

    // true when exiting early
    bool do_exit = false;
    cache_list * result_lst = NULL;

    // Wake curl up..
    if(curl_global_init(CURL_GLOBAL_ALL) != 0)
    {
        glyr_message(-1,NULL,stderr,"Initializing libcurl failed!");
        return NULL;
    }

    // Init the multi handler (=~ container for easy handlers)
    cm = curl_multi_init();

    // we can optionally limit the total amount of connections this multi handle uses
    curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, (long)parallel);
    curl_multi_setopt(cm, CURLMOPT_PIPELINING,  1L);

    // Set up all fields of cb_object (except callback and url)
    for (C = 0; C < CNT; C++)
    {
        char * track = oblist[C].url;

	// format url
        oblist[C].url    = prepare_url(oblist[C].url,oblist[C].s->artist,oblist[C].s->album,oblist[C].s->title);

	// init cache and the CURL handle assicoiated with it
        oblist[C].cache  = handle_init(cm,&oblist[C],s,timeout);

	// old url will be freed as it's always strdup'd
        if(track)
	{
            free(track);
	}

	// > huh?-case
        if(oblist[C].cache == NULL)
        {
            glyr_message(-1,NULL,stderr,"[Internal Error:] Empty callback or empty url!\n");
            glyr_message(-1,NULL,stderr,"[Internal Error:] Call your local C-h4x0r!\n");
            return NULL;
        }
    }

    // counter of plugins tried 
    int n_sources = 0;

    while (U != 0 &&  do_exit == false)
    {
        // Store number of handles still being transferred in U
        CURLMcode merr;
        if( (merr = curl_multi_perform(cm, &U)) != CURLM_OK)
        {
            glyr_message(-1,NULL,stderr,"E: (%s) in dlqueue\n",curl_multi_strerror(merr));
            return NULL;
        }
        if (U != 0)
        {
            FD_ZERO(&R);
            FD_ZERO(&W);
            FD_ZERO(&E);

            if (curl_multi_fdset(cm, &R, &W, &E, &M))
            {
                glyr_message(-1,NULL,stderr, "E: curl_multi_fdset\n");
                return NULL;
            }
            if (curl_multi_timeout(cm, &L))
            {
                glyr_message(-1,NULL,stderr, "E: curl_multi_timeout\n");
                return NULL;
            }
            if (L == -1)
            {
                L = 100;
            }
            if (M == -1)
            {
                DL_usleep(L * 1000);
            }
            else
            {
                T.tv_sec = L/1000;
                T.tv_usec = (L%1000)*1000;

                if (0 > select(M+1, &R, &W, &E, &T))
                {
                    glyr_message(-1,NULL,stderr, "E: select(%i,,,,%li): %i: %s\n",M+1, L, errno, strerror(errno));
                    return NULL;
                }
            }
        }

        while ((msg = curl_multi_info_read(cm, &Q)) && do_exit == false)
        {
            // We're done
            if (msg->msg == CURLMSG_DONE)
            {
                // Get the callback object associated with the curl handle
                // for some odd reason curl requires a char * pointer
                const char * p_pass = NULL;
                CURL *e = msg->easy_handle;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &p_pass);

                // Cast to actual object
                cb_object * capo = (cb_object *) p_pass;

                // The stage is yours <name>
                if(capo->name) glyr_message(2,s,stderr,C_G"[]"C_" %s <%s"C_">...","Trying",capo->name);

                if(capo && capo->cache && capo->cache->data && msg->data.result == CURLE_OK)
                {
                    // Here is where the actual callback shall be executed
                    if(capo->parser_callback)
                    {
                        // Now try to parse what we downloaded
                        cache_list * cl = capo->parser_callback(capo);
			if(cl) 
			{
				if(capo->name)
				{
                        		glyr_message(2,s,stderr,C_G"..found!"C_" (%dx)\n",cl->size);
				}
				else
				{
                        		char c = 0;
					switch(n_sources % 4)
					{
						case 0:
						    c ='\\';
						    break;
						case 1:
						    c ='-';
						    break;
						case 2:
						    c ='/';
						    break;
						case 3:
						    c ='|';
						    break;
					}
                        		glyr_message(2,s,stderr,C_G"O "C_"Downloading [%c] [#%d] - %2.2f\r",c,n_sources,(float)(n_sources / s->number));
				}
			
				if(!result_lst)
				{
				    result_lst = DL_new_lst();
				}
	
				// push all pointers from sublist to resultlist
				DL_push_sublist(result_lst,cl);

				// free the old container
				DL_free_container(cl);

				glyr_message(3,s,stderr,"Tried sources: %d (of max. %d) | Buffers in line: %d\n",n_sources,s->number,result_lst->size);

				// Are we finally done?
				if(n_sources >= CNT || s->number <= result_lst->size)
				{
				    do_exit = true;
				}
			}
                        else if(capo->name)
    	                {
        	            glyr_message(2,s,stderr,C_R"..failed.\n"C_);
                	}
                    }
                    else
                    {
                        glyr_message(1,s,stderr,"WARN: Unable to exec callback (=NULL)\n");
                    }

		    // delete cache 
                    if(capo->cache)
                    {
                        DL_free(capo->cache);
                        capo->cache = NULL;
                    }

                }
                else if(msg->data.result != CURLE_OK)
                {
                    const char * curl_err = curl_easy_strerror(msg->data.result);
                    glyr_message(1,s,stderr,C_R"(!)"C_" CURL ERROR [%d]: %s\n",msg->data.result, curl_err ? curl_err : "Unknown Error");
                }
                else if(capo->cache->data == NULL && capo->name) 
                {
                    glyr_message(2,s,stderr,C_R"..page not reachable.\n"C_);
                }

                curl_multi_remove_handle(cm,e);
                if(capo->url)
                {
                    free(capo->url);
                    capo->url = NULL;
                }

                capo->handle = NULL;
                curl_easy_cleanup(e);
            }
            else
            {
                glyr_message(1,s,stderr, "E: CURLMsg (%d)\n", msg->msg);
            }

            if (C <  CNT)
            {
                // Get a new handle and new cache
                oblist[C].cache = handle_init(cm,&oblist[C],s,timeout);

                C++;
                U++;
            }

	    // amount of tried plugins
            n_sources++;
        }
    }

    // erase "downloading [.] message"
    if(!oblist[0].name)
        glyr_message(2,s,stderr,"%-20c\n",0);

    // Job done - clean up what we did to the memory..
    size_t I = 0;
    for(; I < C; I++)
    {
        DL_free(oblist[I].cache);

        // Free handle
        if(oblist[I].handle != NULL)
            curl_easy_cleanup(oblist[I].handle);

        // Free the prepared URL
        if(oblist[I].url != NULL)
            free(oblist[I].url);

	// erase everything 
	memset(&oblist[I],0,sizeof(cb_object));
    }

    curl_multi_cleanup(cm);
    curl_global_cleanup();
    return result_lst;
}

/*--------------------------------------------------------*/

void plugin_init(cb_object *ref, const char *url, cache_list * (callback)(cb_object*), glyr_settings_t * s, const char *name, void * custom)
{
    // always dup the url
    ref->url = url ? strdup(url) : NULL;

    // Set callback
    ref->parser_callback = callback;

    // empty cache
    ref->cache  = NULL;

    // custom pointer passed to finalize()
    ref->custom = custom;

    // name of plugin (for display)
    ref->name = name;

    // ptr to settings
    ref->s = s;
}

/*--------------------------------------------------------*/

cache_list * register_and_execute(glyr_settings_t * settings, cache_list * (*finalizer)(cache_list *, glyr_settings_t *))
{
    size_t iterator = 0;
    size_t i_plugin = 0;

    plugin_t * providers = settings->providers;
    cache_list * result_lst = NULL;

    // Assume a max. of 10 plugins, just set it higher if you need to.
    cb_object *plugin_list = calloc(sizeof(cb_object), WHATEVER_MAX_PLUGIN);

    while(providers && providers[iterator].name)
    {
        // end of group -> execute
        if(providers[iterator].key == NULL)
        {
	    glyr_message(3,settings,stderr,"Executing jobgroup '%s'\n",providers[iterator].name);

	    // Get the raw result of one jobgroup 
            cache_list * invoked_list = invoke(plugin_list, i_plugin, settings->parallel, settings->timeout, settings);
            if(invoked_list)
            {
		// "finalize" this list
		cache_list * sub_list = finalizer(invoked_list,settings);
		if(sub_list)
		{
			if(!result_lst) result_lst = DL_new_lst();
			// Store pointers to final caches
			DL_push_sublist(result_lst,sub_list);

			// Free all old container
			DL_free_container(sub_list);
		}

		DL_free_lst(invoked_list);
	
		//glyr_message(3,settings,stderr,"Result_list_iterator: %d (up to n=%d)\n",(int)result_lst->size, settings->number);

                if(result_lst != NULL && settings->number <= result_lst->size)
                {
                    // We are done.
		    glyr_message(3,settings,stderr,"register_and_execute() Breaking out.\n");
                    break;
                }
                else
                {
		    // Not enough data -> search again
		    glyr_message(3,settings,stderr,"register_and_execute() (Re)-initializing list.\n");
                    goto RE_INIT_LIST;
                }
            }
            else
            {
		RE_INIT_LIST:
                {
                    // Get ready for next group
                    memset(plugin_list,0,WHATEVER_MAX_PLUGIN * sizeof(cb_object));
                    i_plugin = 0;
                }
            }
        }

        if(providers[iterator].use && providers[iterator].plug.url_callback)
        {
            // Call the Url getter of the plugin
            char * url = (char*)providers[iterator].plug.url_callback(settings);
	    glyr_message(3,settings,stderr,"URL callback return <%s>\n",url);
            if(url)
            {
		glyr_message(3,settings,stderr,"Registering plugin '%s'\n",providers[iterator].name);
                plugin_init( &plugin_list[i_plugin++], url, providers[iterator].plug.parser_callback,settings,providers[iterator].color,NULL);
                if(providers[iterator].plug.free_url)
                {
                    free(url);
                }
            }
        }
        iterator++;
    }

    // Thanks for serving dear list
    if (plugin_list)
    {
        free(plugin_list);
        plugin_list = NULL;
    }

    return result_lst;
}

/*--------------------------------------------------------*/

plugin_t * copy_table(const plugin_t * o, size_t size)
{
    plugin_t * d = malloc(size);
    memcpy(d,o,size);
    return d;
}

/*--------------------------------------------------------*/
