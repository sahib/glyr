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
#include "types.h"
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
                    r = fprintf(stream,"%s",tmp);
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

memCache_t ** DL_new_lst(int n)
{
    memCache_t ** lst = malloc(sizeof(memCache_t*)*(n+1));
    memset(lst,0,sizeof(memCache_t*)*(n+1));
    return lst;
}
/*--------------------------------------------------------*/

void DL_free_lst(memCache_t ** lst, glyr_settings_t *s)
{
    if(lst && s)
    {
        size_t i;
	if(s->type == GET_PHOTO)
	    i = s->photos.offset;
	else
	    i = 0;

        for(; lst[i]; i++)
        {
            DL_free(lst[i]);
        }
        free(lst);
        lst = NULL;
    }
}
/*--------------------------------------------------------*/

// Init an easyhandler with all relevant options
static void DL_setopt(CURL *eh, memCache_t * cache, const char * url, glyr_settings_t * s, void * magic_private_ptr)
{
    if(!s) return;

    // Set options (see 'man curl_easy_setopt')
    curl_easy_setopt(eh, CURLOPT_TIMEOUT, s->timeout);
    curl_easy_setopt(eh, CURLOPT_NOSIGNAL, 1L);

    // last.fm and discogs require an useragent (wokrs without too)
    curl_easy_setopt(eh, CURLOPT_USERAGENT, "BamesJond");
    curl_easy_setopt(eh, CURLOPT_HEADER, 0L);

    // Pass vars to curl
    curl_easy_setopt(eh, CURLOPT_URL, url);
    curl_easy_setopt(eh, CURLOPT_PRIVATE, magic_private_ptr);
    curl_easy_setopt(eh, CURLOPT_VERBOSE, (s->verbosity == 3));
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
static memCache_t * handle_init(CURLM * cm, cb_object * capo, glyr_settings_t *s)
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
    DL_setopt(eh, dlcache, capo->url, s,(void*)capo);

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
        DL_setopt(curl,dldata,url,s,NULL);

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

memCache_t * invoke(cb_object *oblist, long CNT, long parallel, long timeout, glyr_settings_t * s)
{
    CURLM *cm;
    CURLMsg *msg;

    long L;
    unsigned int C = 0;
    int M, Q, U = -1;

    // What we (hopefully) return:
    memCache_t *result = NULL;
    bool do_exit = false;

    // used for select()
    fd_set R, W, E;
    struct timeval T;

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
        oblist[C].url    = prepare_url(oblist[C].url,oblist[C].s->artist,oblist[C].s->album,oblist[C].s->title);
        oblist[C].cache  = handle_init(cm,&oblist[C],s);

        if(track)
        {
            // orig. url always strdup'd
            free(track);
        }

        if(oblist[C].cache == NULL)
        {
            glyr_message(-1,NULL,stderr,"[Internal Error:] Empty callback or empty url!\n");
            glyr_message(-1,NULL,stderr,"[Internal Error:] Call your local C-h4x0r!\n");
            return NULL;
        }
    }

    int progess_indicator = 0;
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

            // No need to check 10k times per second.
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

                const char * p_pass=NULL;
                CURL *e = msg->easy_handle;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &p_pass);

                // Cast to actual object
                cb_object * capo = (cb_object * ) p_pass;

                // The stage is yours <name>
                if(capo->name)
                    glyr_message(2,s,stderr,C_G"[]"C_" %s <%s"C_">...","Trying",capo->name);

                if(capo && capo->cache && capo->cache->data && msg->data.result == CURLE_OK)
                {
                    // Here is where the actual callback shall be executed
                    if(capo->parser_callback)
                    {
                        // Now try to parse what we downloaded
                        result = capo->parser_callback(capo);
			if(result) 
			{
				do_exit = true;
			}
                    }
                    else
                    {
                        // Some bad plugin..
                        result = NULL;
                        glyr_message(1,s,stderr,"WARN: Unable to exec callback (=NULL)\n");
                    }

                    if(capo->cache)
                    {
                        DL_free(capo->cache);
                        capo->cache = NULL;
                    }

                    if(result)
                    {
                        glyr_message(2,s,stderr,C_G"..found!\n"C_);
                    }
                    else if(capo->name)
                    {
                        glyr_message(2,s,stderr,C_R"..failed.\n"C_);
                    }
                    else
                    {
                        char c = 0;
                        switch(progess_indicator % 4)
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
                        glyr_message(2,s,stderr,C_G": "C_"Downloading [%c]\r",c);
                        progess_indicator++;
                    }
                }
                else if(msg->data.result != CURLE_OK)
                {
                    const char * curl_err = curl_easy_strerror(msg->data.result);
                    glyr_message(1,s,stderr,C_R"(!)"C_" CURL ERROR [%d]: %s\n",msg->data.result, curl_err ? curl_err : "Unknown Error");
                }
                else if(capo->cache->data == NULL)
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
                oblist[C].cache = handle_init(cm,&oblist[C],s);

                C++;
                U++;
            }
        }
    }

    // erase "downloading [.] message"
    if(oblist && !oblist[0].name)
        glyr_message(2,s,stderr,"%-20c\n",0);

    // Job done - clean up what we did to the memory..
    size_t I = 0;
    for(; I < C; I++)
    {
        DL_free(oblist[I].cache);

        // Free handle
        if(oblist[I].handle != NULL)
        {
            curl_easy_cleanup(oblist[I].handle);
            oblist[I].handle = NULL;
        }

        // Free the prepared URL
        if(oblist[I].url != NULL)
        {
            free(oblist[I].url);
            oblist[I].url = NULL;
        }

        oblist[I].name = NULL;
        oblist[I].s    = NULL;
    }
    curl_multi_cleanup(cm);
    curl_global_cleanup();


    return result;
}

/*--------------------------------------------------------*/

void plugin_init(cb_object *ref, const char *url, memCache_t*(callback)(cb_object*), glyr_settings_t * s, const char *name, void * custom)
{
    // always dup the url
    ref->url = url?strdup(url):NULL;

    // Set callback
    ref->parser_callback = callback;

    // empty cache
    ref->cache  = NULL;

    // custom pointer passed to finalize()
    ref->custom = custom;

    // name of plugin (for display)
    ref->name   = name;

    // ptr to settings
    ref->s      = s;
}

/*--------------------------------------------------------*/

memCache_t ** register_and_execute(glyr_settings_t * settings, memCache_t**(*finalizer)(memCache_t *, glyr_settings_t *))
{
    size_t i  = 0;
    size_t it = 0;

    plugin_t * providers = settings->providers;

    memCache_t ** result = NULL;

    // Assume a max. of 10 plugins, just set it higher if you need to.
    cb_object *plugin_list = calloc(sizeof(cb_object), WHATEVER_MAX_PLUGIN);

    while(providers && providers[i].name)
    {
        // end of group
        if(providers[i].key == NULL)
        {
            memCache_t * cache = invoke(plugin_list, it, settings->parallel, settings->timeout, settings);
            if(cache)
            {
                if( (result = finalizer(cache, settings)) != NULL)
                {
                    // Oh? We're done?
                    DL_free(cache);
                    break;
                }
                else
                {
                    DL_free(cache);
                    goto RE_INIT_LIST;
                }
            }
            else
            {
RE_INIT_LIST:
                {
                    // Get ready for next group
                    memset(plugin_list,0,WHATEVER_MAX_PLUGIN * sizeof(cb_object));
                    it = 0;
                }
            }
        }
        if(providers[i].use && providers[i].plug.url_callback)
        {
            // Call the Url getter of the plugin
            char * url = (char*)providers[i].plug.url_callback(settings);
            if(url)
            {
                plugin_init( &plugin_list[it++], url, providers[i].plug.parser_callback,settings,providers[i].color,NULL);
                if(providers[i].plug.free_url)
                {
                    free(url);
                }
            }
        }
        i++;
    }

    // call finalize again to make cleaning up in the plugin code possible
    memCache_t ** finalized_result = finalizer(NULL,settings);

    // only update result if finalized_result != NULL
    if(finalized_result != NULL)
    {
        result = finalized_result;
    }

    // Thanks for serving dear list
    if (plugin_list)
    {
        free(plugin_list);
        plugin_list = NULL;
    }

    return result;
}

/*--------------------------------------------------------*/

plugin_t * copy_table(const plugin_t * o, size_t size)
{
    plugin_t * d = malloc(size);
    memcpy(d,o,size);
    return d;
}

/*--------------------------------------------------------*/
