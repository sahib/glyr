/***********************************************************
* This file is part of glyr
* + a commnadline tool and library to download various sort of musicrelated metadata.
* + Copyright (C) [2011]  [Christopher Pahl]
* + Hosted at: https://github.com/sahib/glyr
*
* glyr is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* glyr is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with glyr. If not, see <http://www.gnu.org/licenses/>.
**************************************************************/

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
#else
#include <time.h>
#endif

// All curl stuff we need:
#include <curl/multi.h>

// Own headers:
#include "stringop.h"
#include "core.h"

// Get user agent
#include "config.h"

/*--------------------------------------------------------*/

#define remove_color( string, color )             \
	if(string && color) {                     \
	char * x = strreplace(string,color,NULL); \
	free(string); string = x;}                \
 
int glyr_message(int v, GlyQuery * s, FILE * stream, const char * fmt, ...)
{
    int r = -1;
    if(s || v == -1)
    {
        if(v == -1 || v <= s->verbosity)
        {
            va_list param;
            char * tmp = NULL;
            if(stream && fmt)
            {
                va_start(param,fmt);
                r = x_vasprintf (&tmp, fmt, param);
                va_end(param);

                if(r != -1 && tmp)
                {

// Remove any color if desired
// This feels hackish.. but hey. It's just output.
#ifdef USE_COLOR
                    if(s && !s->color_output)
                    {
                        remove_color(tmp,C_B);
                        remove_color(tmp,C_M);
                        remove_color(tmp,C_C);
                        remove_color(tmp,C_R);
                        remove_color(tmp,C_G);
                        remove_color(tmp,C_Y);
                        remove_color(tmp,C_ );
                    }
#endif
                    r = fprintf(stream,"%s%s",(v>2)?C_B"DEBUG: "C_:"",tmp);
                    free(tmp);
                    tmp = NULL;
                }
            }
        }
    }
    return r;
}

#undef remove_color

/*--------------------------------------------------------*/

// Sleep usec microseconds
static int DL_usleep(long usec)
{
#ifndef WIN32 // posix
    struct timespec req;
    memset(&req,0,sizeof(struct timespec));
    time_t sec = (int) (usec / 1e6L);

    usec = usec - (sec*1e6L);
    req.tv_sec  = (sec);
    req.tv_nsec = (usec*1e3L);

    while(nanosleep(&req , &req) == -1)
    {
        continue;
    }
#else // Windooza 
    Sleep(usec / 1e3L);
#endif
    return 0;
}

/*--------------------------------------------------------*/

GlyMemCache * DL_copy(GlyMemCache * src)
{
    GlyMemCache * dest = NULL;
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

// cache incoming data in a GlyMemCache
static size_t DL_buffer(void *puffer, size_t size, size_t nmemb, void *cache)
{
    size_t realsize = size * nmemb;
    struct GlyMemCache *mem = (struct GlyMemCache *)cache;

    mem->data = realloc(mem->data, mem->size + realsize + 1);
    if (mem->data)
    {
        memcpy(&(mem->data[mem->size]), puffer, realsize);
        mem->size += realsize;
        mem->data[mem->size] = 0;

        if(mem->dsrc && strstr(mem->data,mem->dsrc))
            return 0;
    }
    else
    {
        glyr_message(-1,NULL,stderr,"Caching failed: Out of memory.\n");
        glyr_message(-1,NULL,stderr,"Did you perhaps try to load a 4,7GB iso into your RAM?\n");
    }
    return realsize;
}

/*--------------------------------------------------------*/

// cleanup internal buffer if no longer used
void DL_free(GlyMemCache *cache)
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

        cache->size = 0;
        cache->type = TYPE_NOIDEA;

        free(cache);
        cache = NULL;
    }
}

/*--------------------------------------------------------*/

// Use this to init the internal buffer
GlyMemCache* DL_init(void)
{
    GlyMemCache *cache = malloc(sizeof(GlyMemCache));

    if(cache)
    {
        cache->size     = 0;
        cache->data     = NULL;
        cache->dsrc     = NULL;
        cache->error    = ALL_OK;
        cache->type     = TYPE_NOIDEA;
        cache->duration = 0;
    }
    else
    {
        glyr_message(-1,NULL,stderr,"Warning: empty dlcache...\n");
    }
    return cache;
}

/*--------------------------------------------------------*/

GlyCacheList * DL_new_lst(void)
{
    GlyCacheList * c = calloc(1,sizeof(GlyCacheList));
    if(c != NULL)
    {
        c->size = 0;
        c->usersig = GLYRE_OK;
        c->list = calloc(1,sizeof(GlyMemCache*));
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

void DL_add_to_list(GlyCacheList * l, GlyMemCache * c)
{
    if(l && c)
    {
        l->list = realloc(l->list, sizeof(GlyMemCache *) * (l->size+2));
        l->list[  l->size] = c;
        l->list[++l->size] = NULL;
    }
}

/*--------------------------------------------------------*/

void DL_push_sublist(GlyCacheList * l, GlyCacheList * s)
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
void DL_free_lst(GlyCacheList * c)
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
void DL_free_container(GlyCacheList * c)
{
    if(c!=NULL)
    {
        if(c->list)
            free(c->list);

        memset(c,0,sizeof(GlyCacheList));
        free(c);
    }
}

/*--------------------------------------------------------*/

// Init an easyhandler with all relevant options
static void DL_setopt(CURL *eh, GlyMemCache * cache, const char * url, GlyQuery * s, void * magic_private_ptr, long timeout)
{
    if(!s) return;

    // Set options (see 'man curl_easy_setopt')
    curl_easy_setopt(eh, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(eh, CURLOPT_NOSIGNAL, 1L);

    // last.fm and discogs require an useragent (wokrs without too)
    curl_easy_setopt(eh, CURLOPT_USERAGENT, glyr_VERSION_NAME );
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

    // Do not download 404 pages
    curl_easy_setopt(eh, CURLOPT_FAILONERROR, 1L);

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
static GlyMemCache * handle_init(CURLM * cm, cb_object * capo, GlyQuery *s, long timeout)
{
    // You did sth. wrong..
    if(!capo || !capo->url)
        return NULL;

    // Init handle
    CURL *eh = curl_easy_init();

    // Init cache
    GlyMemCache *dlcache = DL_init();
    if(capo && capo->plug && capo->plug->plug.endmarker)
    {
        dlcache->dsrc = strdup(capo->plug->plug.endmarker);
    }

    // Set handle
    capo->handle = eh;

    // Configure curl
    DL_setopt(eh, dlcache, capo->url, s,(void*)capo,timeout);

    // Add handle to multihandle
    curl_multi_add_handle(cm, eh);
    return dlcache;
}

/*--------------------------------------------------------*/

// return a memcache only with error field set (for error report)
GlyMemCache * DL_error(int eid)
{
    GlyMemCache * ec = DL_init();
    if(ec != NULL)
    {
        ec->error = eid;
    }
    return ec;
}

/*--------------------------------------------------------*/

bool continue_search(int iter, GlyQuery * s)
{
    if(s != NULL)
    {
        if((iter + s->itemctr) < s->number && (iter < s->plugmax || s->plugmax == -1))
        {
            glyr_message(4,s,stderr,"\ncontinue! iter=%d && s->number %d && plugmax=%d && items=%d\n",iter,s->number,s->plugmax,s->itemctr);
            return true;
        }
    }
    glyr_message(4,s,stderr,"\nSTOP! iter=%d && s->number %d && plugmax=%d && items=%d\n",iter,s->number,s->plugmax,s->itemctr);
    return false;
}

/*--------------------------------------------------------*/
// Bad data checker mehods:
/*--------------------------------------------------------*/


int flag_double_urls(GlyCacheList * result, GlyQuery * s)
{
    if(s->duplcheck == false)
        return 0;

    size_t i = 0, dp = 0;
    for(i = 0; i < result->size; i++)
    {
        size_t j = 0;
        if(result->list[i]->error == ALL_OK)
        {
            for(j = 0; j < result->size; j++)
            {
                if(i != j && result->list[i]->error == ALL_OK)
                {
                    if(!strcmp(result->list[i]->data,result->list[j]->data))
                    {
                        result->list[j]->error = DOUBLE_ITEM;
                        dp++;
                    }
                }
            }
        }
    }

    if(dp != 0)
    {
        glyr_message(2,s,stderr,C_R"*"C_" Ignoring %d URL%sthat occure twice.\n",dp,dp>=1 ? " " : "s ");
        s->itemctr -= dp;
    }
    return dp;
}

/*--------------------------------------------------------*/

int flag_invalid_format(GlyCacheList * result, GlyQuery * s)
{
    int c = 0;
    size_t i = 0;
    for(i = 0; i < result->size; i++)
    {
        if(result->list[i]->error != ALL_OK)
            continue;

        char * data = result->list[i]->data;
        size_t dlen = strlen(data);

        if(data != NULL)
        {
            // Search for '.' marking seperator from name
            char * dot_ptr = strrstr_len(data,(char*)".",dlen);
            if(dot_ptr != NULL)
            {
                // Copy format spec
                char * c_format = copy_value(dot_ptr+1,data+dlen);
                if(c_format != NULL)
                {
                    ascii_strdown_modify(c_format);
                    char  * f = NULL;
                    size_t offset = 0, kick_me = false, flen = strlen(s->formats);
                    while(!kick_me && (f = get_next_word(s->formats,";",&offset,flen)))
                    {
                        if(!strcmp(f,c_format))
                        {
                            kick_me = true;
                        }
                        free(f);
                        f = NULL;
                    }

                    if(!kick_me)
                    {
                        result->list[i]->error = BAD_FORMAT;
                        c++;
                    }
                }
                free(c_format);
            }
            else
            {
                result->list[i]->error = BAD_FORMAT;
                c++;
            }
        }
    }
    if(c != 0)
    {
        glyr_message(2,s,stderr,C_R"* "C_"Ignoring %d images with unknown format.\n",c);
        s->itemctr -= c;
    }
    return c;
}

/*--------------------------------------------------------*/

int flag_blacklisted_urls(GlyCacheList * result, const char ** URLblacklist, GlyQuery * s)
{
    if(s->duplcheck == false)
        return 0;

    size_t i = 0, ctr = 0;
    for(i = 0; i < result->size; i++)
    {
        if(result->list[i]->error != ALL_OK)
            continue;

        if(result->list[i]->error == 0)
        {
            size_t j = 0;
            for(j = 0; URLblacklist[j]; j++)
            {
                if(result->list[j]->error == 0)
                {
                    if(!strcmp(result->list[i]->data,URLblacklist[j]))
                    {
                        result->list[i]->error = BLACKLISTED;
                        ctr++;
                    }
                }
            }
        }
    }
    if(ctr != 0)
    {
        glyr_message(2,s,stderr,C_R"*"C_" Ignoring %d blacklisted URL%s.\n\n",ctr,ctr>=1 ? " " : "s ");
        s->itemctr -= ctr;
    }
    return ctr;
}

/*--------------------------------------------------------*/

// Download a singe file NOT in parallel
GlyMemCache * download_single(const char* url, GlyQuery * s, const char * end)
{
    if(url==NULL)
    {
        return NULL;
    }

    //fprintf(stderr,"Downloading <%s>\n",url);

    CURL *curl = NULL;
    CURLcode res = 0;

    // Init handles
    curl = curl_easy_init();
    GlyMemCache * dldata = DL_init();
    if(end != NULL)
    {
        dldata->dsrc = strdup(end);
    }

    if(curl != NULL)
    {
        // Configure curl
        DL_setopt(curl,dldata,url,s,NULL,s->timeout);

        // Perform transaction
        res = curl_easy_perform(curl);

        // Better check again
        if(res != CURLE_OK && res != CURLE_WRITE_ERROR)
        {
            glyr_message(-1,NULL,stderr,C_"\n:: %s [E:%d]\n", curl_easy_strerror(res),res);
            DL_free(dldata);
            dldata = NULL;
        }

        // Handle without any use - clean up
        curl_easy_cleanup(curl);
        return dldata;
    }

    // Free mem
    DL_free(dldata);
    return NULL;
}

/*--------------------------------------------------------*/

static void print_progress(int status, GlyQuery * s)
{
    char c = 0;
    switch(status % 4)
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
    glyr_message(2,s,stderr,C_"Downloading [%c] [#%d]\r",c,status+1);
}


// macro to align console output..
#define ALIGN(m)                      \
if(m > 0) {                           \
     int i = 0;                       \
     int d = ABS(20-m);               \
     glyr_message(2,s,stderr,C_);     \
     for(;i<d;i++) {                  \
        glyr_message(2,s,stderr,"."); \
     }  }                             \
 
GlyCacheList * invoke(cb_object *oblist, long CNT, long parallel, long timeout, GlyQuery * s)
{
    // curl multi handles
    CURLM *cm;
    CURLMsg *msg;

    // select()
    long L;
    unsigned int Counter = 0;

    // select() related stuff
    int M, Q, U = -1;
    fd_set ReadFDS, WriteFDS, ErrorFDS;
    struct timeval Tmax;

    // true when exiting early
    bool do_exit = false;
    GlyCacheList * result_lst = NULL;

    // Init the multi handler (=~ container for easy handlers)
    cm = curl_multi_init();

    // we can optionally limit the total amount of connections this multi handle uses
    curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, (long)parallel);
    curl_multi_setopt(cm, CURLMOPT_PIPELINING,  1L);

    // Set up all fields of cb_object (except callback and url)
    for (Counter = 0; Counter < CNT; Counter++)
    {
        char * track = oblist[Counter].url;
        printf("%p:%s | %p\n",oblist[Counter].url,oblist[Counter].url,oblist[Counter].s);
        oblist[Counter].url    = prepare_url(oblist[Counter].url,
                                             oblist[Counter].s->artist,
                                             oblist[Counter].s->album,
                                             oblist[Counter].s->title);
        oblist[Counter].cache  = handle_init(cm,&oblist[Counter],s,timeout);
        if(track)
        {
            free(track);
            track = NULL;
        }
        if(oblist[Counter].cache == NULL || oblist[Counter].url == NULL)
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
            FD_ZERO(&ReadFDS);
            FD_ZERO(&WriteFDS);
            FD_ZERO(&ErrorFDS);

            if (curl_multi_fdset(cm, &ReadFDS, &WriteFDS, &ErrorFDS, &M))
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
                DL_usleep(L * 100);
            }
            else
            {
                Tmax.tv_sec = L/1000;
                Tmax.tv_usec = (L%1000)*1000;

                if (select(M+1, &ReadFDS, &WriteFDS, &ErrorFDS, &Tmax) == -1)
                {
                    glyr_message(-1,NULL,stderr, "E: select(%i ... %li): %i: %s\n",M+1, L, errno, strerror(errno));
                    return NULL;
                }
            }
        }

        while (do_exit == false && (msg = curl_multi_info_read(cm, &Q)))
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

                int align_msg = 0;
                if(!capo->batch)
                {
                    align_msg = strlen(capo->plug->name);
                    glyr_message(2,s,stderr,"Query: %s",capo->plug->color);
                }

                if(capo && capo->cache && capo->cache->data && (msg->data.result == CURLE_OK || msg->data.result == CURLE_WRITE_ERROR))
                {
                    // Here is where the actual callback shall be executed
                    if(capo->parser_callback)
                    {
                        // Now try to parse what we downloaded
                        GlyCacheList * cl = capo->parser_callback(capo);
                        if(cl && cl->list && cl->list[0]->data)
                        {
                            if(!capo->batch)
                            {
                                ALIGN(align_msg);
                                glyr_message(2,s,stderr,C_G"found!"C_" ["C_R"%d item%c"C_"%s]\n",cl->size,cl->size == 1 ? ' ' : 's',cl->size>9 ? "" : " ");
                                s->itemctr += cl->size;
                            }
                            else
                            {
                                print_progress(n_sources, s);
                            }

                            if(!result_lst)
                            {
                                result_lst = DL_new_lst();
                            }


                            glyr_message(3,s,stderr,"Tried sources: %d (of max. %d) | Buffers in line: %d\n",n_sources,s->number,result_lst->size);

                            // Are we finally done?
                            if((capo->batch && cl->usersig == GLYRE_STOP_BY_CB) || n_sources >= CNT ||
                                    (!capo->batch ? s->number <= s->itemctr : s->number <= (int)result_lst->size))
                            {
                                do_exit = true;
                            }

                            if((capo->batch && cl->usersig == GLYRE_OK) || !capo->batch)
                            {
                                // push all pointers from sublist to resultlist
                                DL_push_sublist(result_lst,cl);
                            }

                            // free the old container
                            DL_free_container(cl);
                        }
                        else if(!capo->batch)
                        {
                            ALIGN(align_msg);
                            int err = (!cl) ? -1 : cl->list[0]->error;
                            switch(err)
                            {
                            case NO_BEGIN_TAG:
                                glyr_message(2,s,stderr,C_R"No begin tag found.\n"C_);
                                break;
                            case NO_ENDIN_TAG:
                                glyr_message(2,s,stderr,C_R"No endin tag found.\n"C_);
                                break;
                            default:
                                glyr_message(2,s,stderr,C_R"failed.\n"C_);
                            }
                            if(cl != NULL) DL_free_lst(cl);
                        }
                        else if(capo->batch)
                        {
                            // If NULL is returned this means usually
                            // that GLYRE_STOP_BY_CB was returned
                            // so we stop...
                            do_exit = true;
                        }
                    }
                    else
                    {
                        ALIGN(align_msg);
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
                    ALIGN(align_msg);
                    const char * curl_err = curl_easy_strerror(msg->data.result);
                    glyr_message(1,s,stderr,"#[%d]"C_R" %s.\n"C_,msg->data.result,curl_err ? curl_err : "Unknown Error");
                }
                else
                {
                    ALIGN(align_msg);
                    glyr_message(2,s,stderr,C_R"failed.\n"C_);
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
            else glyr_message(1,s,stderr, "E: CURLMsg (%d)\n", msg->msg);

            if (Counter <  CNT)
            {
                // Get a new handle and new cache
                oblist[Counter].cache = handle_init(cm,&oblist[Counter],s,timeout);

                Counter++;
                U++;
            }

            // amount of tried plugins
            n_sources++;
        }
    }

    // erase "downloading [.] message"
    if(oblist[0].batch) glyr_message(2,s,stderr,"%-25c\n",1);
    size_t I = 0;
    for(I = 0; I < Counter; I++)
    {
        DL_free(oblist[I].cache);
        oblist[I].cache = NULL;

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
        memset(&oblist[I],0,sizeof(cb_object));
    }
    curl_multi_cleanup(cm);
    return result_lst;
}

/*--------------------------------------------------------*/

void plugin_init(cb_object *ref, const char *url, GlyCacheList * (callback)(cb_object*), GlyQuery * s, GlyPlugin * plug, const char * endmark, bool batch)
{
    ref->url = url ? strdup(url) : NULL;
    ref->parser_callback = callback;
    ref->cache  = NULL;
    ref->plug = plug;
    ref->batch = batch;
    ref->endmark = endmark;
    ref->s = s;
    ref->handle = NULL;
}

/*--------------------------------------------------------*/

GlyCacheList * register_and_execute(GlyQuery * query, GlyCacheList * (*finalizer)(GlyCacheList *, GlyQuery *))
{
    // Get the providers
    GlyPlugin * pList = query->providers;
    if(pList == NULL)
    {
        glyr_message(-1,NULL,stderr,C_R"FATAL:"C_" Empty provider. Unable to register nor execute!\n");
        glyr_message(-1,NULL,stderr,C_R"FATAL:"C_" This should not happen as this should catched before.\n");
        return NULL;
    }

    // A container storing all URLs that specified plugins need to work
    // (They are downlaoded by invoke() and made available to the plugin via a cb_oject called 'capo')
    cb_object *URLContainer   = NULL;
    GlyCacheList * resultList = NULL;

    // Now jsut register all Plugins that are there
    // This is for simplicity reasons, and does not take
    // away any ressources...
    size_t iter = 0, plugCtr = 0;
    for(iter = 0; pList[iter].name != NULL; ++iter)
    {
        // Call the URLcallback of the plugin
        char * pUrl = (char*)pList[iter].plug.url_callback(query);
        if(pUrl != NULL)
        {
            // make debuggin little easier
            glyr_message(3,query,stderr,"plugin=%s | url=<%s>\n",pList[iter].name,pUrl);

            // enlarge URLContainer
            URLContainer = realloc(URLContainer, sizeof(cb_object) * (plugCtr+1));
            if(URLContainer == NULL)
            {
                glyr_message(-1,NULL,stderr,C_R"FATAL:"C_" No memory to allocate URLContainer!\n");
                URLContainer = NULL;
                break;
            }

            plugin_init( &URLContainer[plugCtr], pUrl, pList[iter].plug.parser_callback, query, &pList[iter], pList[iter].plug.endmarker, false);

            // If the plugin uses dynamic memory it should set plug.free_url to TRUE
            // So it is free'd here. plugin_init strdups the URL and invoke handles all URLs equally.
            if(pList[iter].plug.free_url)
            {
                free(pUrl);
                pUrl = NULL;
            }
            // Make sure we don't use $iter as URLContainer iterator (creepy consequences if URLCallbacks returns NULL)
            plugCtr++;
        }
    }
    if(plugCtr != 0)
    {
        // Internal Order in whic we visit groups
        int GIDArray[] = {GRP_SAFE,  GRP_FAST, GRP_USFE,  GRP_SPCL,  GRP_SLOW,  -666 /* Negative evil */};
        // Mark a plugin if visited if it's in multiple groups
        bool * visitList = calloc(plugCtr,sizeof(char));
        if(visitList == NULL)
        {
            glyr_message(-1,NULL,stderr,"NoMemoryException!\n");
            return NULL;
        }

        // Iterate through each group and makes sure we don't overiterate
        for(iter = 0; GIDArray[iter] != -666 && query->itemctr < query->number; iter++)
        {
            size_t inner = 0, invCtr = 0;
            cb_object * invokeList = NULL;
            for(inner = 0; inner < plugCtr; inner++)
            {
                bool take_group = (query->groupedDL) ? (URLContainer[inner].plug->gid & GIDArray[iter]) : true;
                if(URLContainer[inner].plug->use && take_group && visitList[inner] == false)
                {
                    // Mark Plugin
                    visitList[inner] = true;

                    // Let
                    invokeList = realloc(invokeList,sizeof(cb_object) * (invCtr+1));
                    memcpy(&invokeList[invCtr],&URLContainer[inner],sizeof(cb_object));
                    invCtr++;
                }
            }

            if(invCtr > 0)
            {
                const char * group_name = (query->groupedDL) ? grp_id_to_name(GIDArray[iter]) : "all";
                glyr_message(2,query,stderr,"Invoking group "C_Y"%s"C_":\n",group_name);

                // Get the items from invoke()
                GlyCacheList * dlData = invoke(invokeList, invCtr, query->parallel, query->timeout, query);

                if(dlData != NULL)
                {
                    // Call the finalize call, so the items get validated and whatever
                    GlyCacheList * subList = finalizer(dlData,query);
                    if(subList != NULL)
                    {
                        glyr_message(3,query,stderr,"Adding %d items to resultList\n",subList->size);
                        if(resultList == NULL)
                        {
                            resultList = DL_new_lst();
                        }
                        DL_push_sublist(resultList,subList);
                        DL_free_container(subList);
                    }
                    DL_free_lst(dlData);
                }
            }
            free(invokeList);
            invokeList = NULL;
        }

        // Free URLs that have been allocated, but not used
        size_t I = 0;
        for(I = 0; I < plugCtr; I++)
        {
            if(visitList[I]==false && URLContainer[I].url)
            {
                free(URLContainer[I].url);
                URLContainer[I].url = NULL;
            }
        }
        free(visitList);
        visitList = NULL;

    }
    else glyr_message(2,query,stderr,"Sorry. No Plugin were registered, so nothing was downloaded... :(\n");

    // allow plugins to cleanup
    finalizer(NULL,query);

    // Free the Container.
    if(URLContainer != NULL) free(URLContainer);
    return resultList;
}

/*--------------------------------------------------------*/

GlyPlugin * copy_table(const GlyPlugin * o, size_t size)
{
    GlyPlugin * d = malloc(size);
    memcpy(d,o,size);
    return d;
}

/*--------------------------------------------------------*/

const char * grp_id_to_name(int id)
{
    switch(id)
    {
    case GRP_SAFE:
        return GRPN_SAFE;
    case GRP_USFE:
        return GRPN_USFE;
    case GRP_FAST:
        return GRPN_FAST;
    case GRP_SLOW:
        return GRPN_SLOW;
    case GRP_SPCL:
        return GRPN_SPCL;
    case GRP_NONE:
        return GRPN_NONE;
    case GRP_ALL:
        return GRPN_ALL;
    default:
        return NULL;
    }
    return NULL;
}

/*--------------------------------------------------------*/
