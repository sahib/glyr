#include <errno.h>
#include <stdlib.h>
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

/*--------------------------------------------------------*/

// Sleep usec microseconds
int DL_usleep(long usec)
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

    // <!! STATIC WARNING !!>
    static char print_dot;
    if( (print_dot = (print_dot == 2) ? 0 :
                     (print_dot == 1) ? 2 : 1) == 1)
    {
        //	fprintf(stderr,(mem->ecode == -1) ? C_G""C_  : C_M":"C_);
    }
    // </!! STATIC WARNING !!>

    mem->data = realloc(mem->data, mem->size + realsize + 1);
    if (mem->data)
    {
        memcpy(&(mem->data[mem->size]), puffer, realsize);
        mem->size += realsize;
        mem->data[mem->size] = 0;
    }
    else
    {
        fprintf(stderr,"Caching failed: Out of memory.\n");
    }
    return realsize;
}

/*--------------------------------------------------------*/

// cleanup internal buffer if no longer used
void DL_free(memCache_t *cache)
{
    if(cache != NULL)
    {
        if(cache->size && cache->data != NULL)
        {
            free(cache->data);
        }

        cache->size   = 0;
        cache->ecode  = 0;
        cache->data   = NULL;

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
        cache->ecode  = 0;
        cache->data   = NULL;
    }
    else
    {
        fprintf(stderr,"Warning: empty dlcache...\n");
    }
    return cache;
}

/*--------------------------------------------------------*/

// Init an easyhandler with all relevant options
static void DL_setopt(CURL *eh, memCache_t * cache, const char * url, long timeout, long redirects, void * magic_private_ptr)
{
    // Set options (see 'man curl_easy_setopt')
    curl_easy_setopt(eh, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(eh, CURLOPT_NOSIGNAL, 1L);

    // last.fm and discogs require an useragent (wokrs without too)
    curl_easy_setopt(eh, CURLOPT_USERAGENT, "firefox");
    curl_easy_setopt(eh, CURLOPT_HEADER, 0L);

    // Pass vars to curl
    curl_easy_setopt(eh, CURLOPT_URL, url);
    curl_easy_setopt(eh, CURLOPT_PRIVATE, magic_private_ptr);
    curl_easy_setopt(eh, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, DL_buffer);
    curl_easy_setopt(eh, CURLOPT_WRITEDATA, (void *)cache);

    // amazon plugin requires redirects
    curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(eh, CURLOPT_MAXREDIRS, redirects);

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
static memCache_t * handle_init(CURLM * cm, cb_object * caps, long timeout, long redirects)
{
    CURL *eh = curl_easy_init();

    // You did sth. wrong..
    if(caps == NULL || caps->url == NULL)
    {
        return NULL;
    }

    // Init cache
    memCache_t *dlcache = DL_init();

    // Set handle
    caps->handle = eh;

    // Configure curl
    DL_setopt(eh, dlcache, caps->url, timeout, redirects,(void*)caps);

    // Add handle to multihandle
    curl_multi_add_handle(cm, eh);
    return dlcache;
}

/*--------------------------------------------------------*/

// Download a singe file NOT in parallel
memCache_t * download_single(const char* url, long redirects)
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
        fprintf(stderr,"?? libcurl failed to init.\n");
    }

    // Init handles
    curl = curl_easy_init();
    memCache_t * dldata = DL_init();
    dldata->ecode = -1;

    if(curl != NULL)
    {
        // Configure curl
        DL_setopt(curl,dldata,url,20L,redirects,NULL);

        // Perform transaction
        res = curl_easy_perform(curl);

        // Better check again
        if(res != CURLE_OK)
        {
            fprintf(stderr,C_"\n:: %s\n", curl_easy_strerror(res));
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

memCache_t * invoke(cb_object *oblist, long CNT, long parallel, long timeout, long redirects, const char *artist, const char* album, const char *title, const char * status)
{
    CURLM *cm;
    CURLMsg *msg;

    long L;
    unsigned int C = 0;
    int M, Q, U = -1;

    // What we (hopefully) return:
    memCache_t *result = NULL;

    // used for select()
    fd_set R, W, E;
    struct timeval T;

    // Wake curl up..
    if(curl_global_init(CURL_GLOBAL_ALL) != 0)
    {
	printf("Init failed!");
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

        oblist[C].artist = artist;
        oblist[C].album  = album;
        oblist[C].title  = title;
        oblist[C].url    = prepare_url(oblist[C].url,oblist[C].artist,oblist[C].album,oblist[C].title);

        oblist[C].cache = handle_init(cm,&oblist[C],timeout,redirects);

        if(oblist[C].cache == NULL)
        {
            fprintf(stderr,"[Internal Error:] Empty callback or empty url!\n");
            fprintf(stderr,"[Internal Error:] Call your local C-h4x0r!\n");
            return NULL;
        }
    }
    while (U != 0 && result == NULL)
    {
        // Store number of handles still being transferred in U
        CURLMcode merr;
        if( (merr = curl_multi_perform(cm, &U)) != CURLM_OK)
        {
            fprintf(stderr,"E: (%s) in dlqueue\n",curl_multi_strerror(merr));
            return NULL;
        }

        if (U != 0)
        {
            FD_ZERO(&R);
            FD_ZERO(&W);
            FD_ZERO(&E);

            if (curl_multi_fdset(cm, &R, &W, &E, &M))
            {
                fprintf(stderr, "E: curl_multi_fdset\n");
                return NULL;
            }

            if (curl_multi_timeout(cm, &L))
            {
                fprintf(stderr, "E: curl_multi_timeout\n");
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
                    fprintf(stderr, "E: select(%i,,,,%li): %i: %s\n",M+1, L, errno, strerror(errno));
                    return NULL;
                }
            }
        }

        while ((msg = curl_multi_info_read(cm, &Q)) && result == NULL)
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
                fprintf(stderr,"%s"C_G":"C_" %s <%s"C_">...%c",capo->url,(status) ? status : "Trying",capo->name,(status) ? '\n' : '\0');
                if(capo && capo->cache && capo->cache->data && msg->data.result == 0)
                {
                    // Here is where the actual callback shall be executed
                    if(capo->parser_callback)
                    {
                        // Now try to parse what we downloaded
                        result = capo->parser_callback(capo);
                    }
                    else
                    {
                        // Some bad plugin..
                        result = NULL;
                        fprintf(stderr,"WARN: Unable to exec callback (=NULL)\n");
                    }

                    if(capo->cache)
                    {
                        DL_free(capo->cache);
                        capo->cache = NULL;
                    }

		    if(!status)
		    {
			    if(result)
			    {
				fprintf(stderr,C_G"..found!\n"C_);
			    }
			    else
			    {
				fprintf(stderr,C_R"..failed.\n"C_);
			    }
		    }
                }
		else 
		{
			const char * curl_err = curl_easy_strerror(msg->data.result);
			printf(C_R"(!)"C_" ERROR [%d]: %s\n",msg->data.result, curl_err ? curl_err : "Unknown Error");
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
                fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
            }

            if (C <  CNT)
            {
                // Get a new handle and new cache
                oblist[C].cache = handle_init(cm,&oblist[C],timeout,redirects);

                C++;
                U++;
            }
        }
    }

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

        // Make list the same as before
        oblist[I].artist = NULL;
        oblist[I].album  = NULL;
        oblist[I].title  = NULL;
        oblist[I].name   = NULL;
        oblist[I].min    = 0;
        oblist[I].max    = 0;
    }
    curl_multi_cleanup(cm);
    curl_global_cleanup();

    return result;
}

/*--------------------------------------------------------*/

void plugin_init(cb_object *ref, const char *url, memCache_t*(callback)(cb_object*), int min, int max, const char *name)
{
    // Set url (and make it nicely readable for curl)
    ref->url = (char*)url;

    // Set callback
    ref->parser_callback = callback;

    // Gets filled by invoke()
    ref->cache  = NULL;
    ref->artist = NULL;
    ref->album  = NULL;
    ref->handle = NULL;
    ref->title  = NULL;

    // Set mins/max
    ref->max  = max;
    ref->min  = min;
    ref->name = name;
}

/*--------------------------------------------------------*/

int write_file(const char *path, memCache_t *data)
{
    // dumb write
    if(path == NULL || data == NULL)
    {
        return -1;
    }
    else
    {
        FILE *pF = fopen(path,"w");

        if(pF)
        {
            int b = fwrite(data->data,sizeof(char),data->size,pF);
            fclose(pF);
            return b;
        }
    }
    return -1;
}

/*--------------------------------------------------------*/

int sk_is_in(sk_pair_t * arr, const char * string)
{
    size_t i = 0;

    while(arr[i].name != NULL)
    {
        if(!strcasecmp(arr[i].name,string) || (arr[i].key && !strcasecmp(arr[i].key,string)))
        {
            arr[i].use = true;
            return i;
        }

        i++;
    }

    printf(": Unknown word: '%s'\n",string);

    i = 0;
    char flag = 1;
    while(arr[i].name != NULL)
    {
        if(levenshtein_strcmp(arr[i].name,string) < 5)
        {
            if(flag)
            {
                printf(": Did you mean ");
            }

            printf("%c'%s' ",(flag) ? ' ' : ',',arr[i].name);

            if(flag) flag = 0;
        }
        i++;
    }

    if(!flag) printf("?\n");
    return -1;
}

/*--------------------------------------------------------*/

const char * register_and_execute(glyr_settings_t * settings, const char * filename, const char * (* finalizer) (cb_object *, size_t it, const char *, glyr_settings_t *))
{
    size_t i = 0;
    size_t it   = 0;

    // Sanitize input
    int max = settings->cover_max_size;
    int min = settings->cover_min_size;
    if (max != -1 && min > max) min=-1;
    if (min != -1 && max < min) max=-1;

    sk_pair_t * providers = settings->providers;

    const char * result = NULL;

    // Assume a max. of 10 plugins, just set it higher if you need to.
    cb_object *plugin_list = malloc(sizeof(cb_object) * COVER_MAX_PLUGIN);

    while(providers && providers[i].name)
    {
        // end of group
        if(providers[i].key == NULL)
        {
	    //printf("Executing grp %s",providers[i].name);
            if( (result = finalizer(plugin_list, it, filename, settings)) == NULL)
            {
                // Get ready for next group
                memset(plugin_list,0,COVER_MAX_PLUGIN * sizeof(cb_object));
                it = 0;
            }
            else
            {
                // Oh? We're done?
                break;
            }
        }
        if(providers[i].use && providers[i].plug.url_callback)
        {
            // Call the Url getter of the plugin
            const char * url = providers[i].plug.url_callback(settings);
            if(url)
            {
                plugin_init( &plugin_list[it++], providers[i].plug.url_callback(settings), providers[i].plug.parser_callback,min,max,providers[i].color);

                if(providers[i].plug.free_url && url)
                {
                    free((char*)url);
                }
            }
        }
        i++;
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
