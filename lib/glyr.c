/***********************************************************
 * This file is part of glyr
 * + a commnandline tool and library to download various sort of musicrelated metadata.
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

#include <glib.h>
#include <locale.h>

#include "glyr.h"
#include "core.h"
#include "register_plugins.h"
#include "blacklist.h"
#include "md5.h"

#if glyr_DEBUG
#ifndef WIN32
/* Backtrace*/
#include <execinfo.h>
#endif
#endif

//* ------------------------------------------------------- */

gboolean is_initalized = FALSE;

//* ------------------------------------------------------- */

#if glyr_DEBUG
#ifndef WIN32
#define STACK_FRAME_SIZE 20
/* Obtain a backtrace and print it to stdout. */
static void print_trace(void)
{
    void * array[STACK_FRAME_SIZE];
    gchar ** bt_info_list;
    gsize size, it = 0;

    size = backtrace (array, STACK_FRAME_SIZE);
    bt_info_list = backtrace_symbols(array, size);

    for (it = 0; it < size; it++)
    {
        g_printerr("    [#%02u] %s\n",(gint)it+1, bt_info_list[it]);
    }

    g_printerr("\n%zd calls in total are shown.\n", size);
    g_free(bt_info_list);
}
#endif

//* ------------------------------------------------------- */

static void sig_handler(int signal)
{
    switch(signal)
    {
    case SIGABRT :
    case SIGFPE  :
    case SIGSEGV : /* sigh */
        glyr_message(-1,NULL,C_R"\nFATAL: "C_"libglyr stopped/crashed due to a %s of death.\n",g_strsignal(signal));
        glyr_message(-1,NULL,C_"       This is entirely the fault of the libglyr developers. Yes, we failed. Sorry. Now what to do:\n");
        glyr_message(-1,NULL,C_"       It would be just natural to blame us now, so just visit <https://github.com/sahib/glyr/issues>\n");
        glyr_message(-1,NULL,C_"       and throw hard words like 'backtrace', 'bug report' or even the '$(command I issued' at them).\n");
        glyr_message(-1,NULL,C_"       The libglyr developers will try to fix it as soon as possible so you stop pulling their hair.\n");
#ifndef WIN32
        glyr_message(-1,NULL,C_"\nA list of the last called functions follows, please add this to your report:\n");
        print_trace();
#endif
        glyr_message(-1,NULL,C_"\n(Thanks, and Sorry for any bad feelings.)\n\n");
        break;
    }
    exit(EXIT_FAILURE);
}
#endif

/*--------------------------------------------------------*/

const char * err_strings[] =
{
    "Unknown Error",                                   /* GLYRE_UNKNOWN      */
    "Everything is fine",                              /* GLYRE_OK           */
    "Bad option for gly_opt_*()",                      /* GLYRE_BAD_OPTION   */
    "Bad value for glyr_opt_*()",                      /* GLYRE_BAD_VALUE    */
    "Empty Query structure (NULL)",                    /* GLYRE_EMPTY_STRUCT */
    "No provider specified in glyr_opt_from()",        /* GLYRE_NO_PROVIDER  */
    "Unknown GET_TYPE in glyr_get()",                  /* GLYRE_UNKNOWN_GET  */
    "Cache was skipped due to user",                   /* GLYRE_SKIP         */
    "Stopped by callback (post)",                      /* GLYRE_STOP_POST    */
    "Stopped by callback (pre)",                       /* GLYRE_STOP_PRE     */
    "Library is not yet initialized, use glyr_init()", /* GLYRE_NO_INIT      */
    NULL
};

/*--------------------------------------------------------*/

static gchar * auto_choose_lang(GlyrQuery * query)
{
    gchar * result_lang = NULL;
    const gchar * const * possible_locales = g_get_language_names ();
    if(possible_locales != NULL)
    {
        gchar * wanted = "en";
        gsize min_length = INT_MAX;
        for(gint i = 0; possible_locales[i]; i++)
        {
            gsize loc_len = strlen(possible_locales[i]);
            if(loc_len < min_length && g_ascii_strncasecmp("C",possible_locales[i],1) != 0)
            {
                wanted = (gchar*)possible_locales[i];
                min_length = loc_len;
            }
        }
        result_lang = g_strdup(wanted);
        glyr_message(2,query,"- Language : %s\n",result_lang);
    }
    g_strfreev((gchar**)possible_locales);
    return result_lang;
}

/*--------------------------------------------------------*/

static int glyr_set_info(GlyrQuery * s, int at, const char * arg);
static void set_query_on_defaults(GlyrQuery * glyrs);

/*--------------------------------------------------------*/
/*-------------------- OTHER -----------------------------*/
/*--------------------------------------------------------*/

GlyrMemCache * glyr_copy_cache(GlyrMemCache * source)
{
    return DL_copy(source);
}

/*--------------------------------------------------------*/

// return a descriptive string on error ID
const char * glyr_strerror(enum GLYR_ERROR ID)
{
    if(ID < (sizeof(err_strings)/sizeof(const char *)))
    {
        return err_strings[ID];
    }
    return err_strings[0];
}

/*-----------------------------------------------*/

void glyr_update_md5sum(GlyrMemCache * cache)
{
    update_md5sum(cache);
}

/*-----------------------------------------------*/

const char * glyr_version(void)
{
    return "Version "glyr_VERSION_MAJOR"."glyr_VERSION_MINOR" ("glyr_VERSION_NAME") of ["__DATE__"] compiled at ["__TIME__"]";
}

/*-----------------------------------------------*/
// _opt_
/*-----------------------------------------------*/

// Seperate method because va_arg struggles with function pointers
enum GLYR_ERROR glyr_opt_dlcallback(GlyrQuery * settings, DL_callback dl_cb, void * userp)
{
    if(settings)
    {
        settings->callback.download     = dl_cb;
        settings->callback.user_pointer = userp;
        return GLYRE_OK;
    }
    return GLYRE_EMPTY_STRUCT;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_type(GlyrQuery * s, enum GLYR_GET_TYPE type)
{

    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    if(type < GET_UNSURE)
    {
        s->type = type;
        return GLYRE_OK;
    }
    return GLYRE_BAD_VALUE;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_artist(GlyrQuery * s, char * artist)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    glyr_set_info(s,0,artist);
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_album(GlyrQuery * s, char * album)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    glyr_set_info(s,1,album);
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_title(GlyrQuery * s, char * title)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    glyr_set_info(s,2,title);
    return GLYRE_OK;
}

/*-----------------------------------------------*/

static int size_set(int * ref, int size)
{
    if(size < -1 && ref)
    {
        *ref = -1;
        return GLYRE_BAD_VALUE;
    }

    if(ref)
    {
        *ref = size;
        return GLYRE_OK;
    }
    return GLYRE_BAD_OPTION;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_img_maxsize(GlyrQuery * s, int size)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    return size_set(&s->img_max_size,size);
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_img_minsize(GlyrQuery * s, int size)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    return size_set(&s->img_min_size,size);
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_parallel(GlyrQuery * s, unsigned long val)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->parallel = (long)val;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_timeout(GlyrQuery * s, unsigned long val)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->timeout = (long)val;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_redirects(GlyrQuery * s, unsigned long val)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->redirects = (long)val;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_useragent(GlyrQuery * s, const char * useragent)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    glyr_set_info(s,6,(useragent) ? useragent : "");
    return GLYRE_OK;
}

/*-----------------------------------------------*/

#define AUTO_STRING "auto"
enum GLYR_ERROR glyr_opt_lang(GlyrQuery * s, char * langcode)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    if(langcode != NULL)
    {
        if(g_ascii_strncasecmp(AUTO_STRING,langcode,(sizeof AUTO_STRING) - 1) == 0)
        {
            gchar * auto_lang = auto_choose_lang(s);
            glyr_set_info(s,7,auto_lang);
            g_free(auto_lang);
        }
        else
        {
            glyr_set_info(s,7,langcode);
        }
        return GLYRE_OK;
    }
    return GLYRE_BAD_VALUE;
}
#undef AUTO_STRING

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_number(GlyrQuery * s, unsigned int num)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->number = num == 0 ? INT_MAX : num;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_verbosity(GlyrQuery * s, unsigned int level)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->verbosity = level;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_from(GlyrQuery * s, const char * from)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    if(from != NULL)
    {
        glyr_set_info(s,4,from);
        return GLYRE_OK;
    }
    return GLYRE_BAD_VALUE;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_allowed_formats(GlyrQuery * s, const char * formats)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    glyr_set_info(s,5,(formats==NULL) ? DEFAULT_ALLOWED_FORMATS : formats);
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_plugmax(GlyrQuery * s, int plugmax)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    if(plugmax < 0)
    {
        return GLYRE_BAD_VALUE;
    }

    s->plugmax = plugmax;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_qsratio(GlyrQuery * s, float ratio)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->qsratio = MIN(MAX(ratio,0.0),1.0);
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_proxy(GlyrQuery * s, const char * proxystring)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    glyr_set_info(s,3,proxystring);
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_fuzzyness(GlyrQuery * s, int fuzz)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->fuzzyness = fuzz;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_download(GlyrQuery * s, bool download)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->download = download;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_force_utf8(GlyrQuery * s, bool force_utf8)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->force_utf8 = force_utf8;
    return GLYRE_OK;
}

/*-----------------------------------------------*/
/*-----------------------------------------------*/
/*-----------------------------------------------*/

static void set_query_on_defaults(GlyrQuery * glyrs)
{
    /* Initialize free pointer pool */
    memset(glyrs,0,sizeof(GlyrQuery));

    glyrs->type = GET_UNSURE;
    glyrs->artist = NULL;
    glyrs->album  = NULL;
    glyrs->title  = NULL;
    glyrs->from   = NULL;
    glyrs->callback.download = NULL;
    glyrs->callback.user_pointer = NULL;

    glyrs->img_min_size = DEFAULT_CMINSIZE;
    glyrs->img_max_size = DEFAULT_CMAXSIZE;
    glyrs->number = DEFAULT_NUMBER;
    glyrs->parallel  = DEFAULT_PARALLEL;
    glyrs->redirects = DEFAULT_REDIRECTS;
    glyrs->timeout   = DEFAULT_TIMEOUT;
    glyrs->verbosity = DEFAULT_VERBOSITY;
    glyrs->lang = DEFAULT_LANG;
    glyrs->plugmax = DEFAULT_PLUGMAX;
    glyrs->download = DEFAULT_DOWNLOAD;
    glyrs->fuzzyness = DEFAULT_FUZZYNESS;
    glyrs->proxy = DEFAULT_PROXY;
    glyrs->qsratio = DEFAULT_QSRATIO;
    glyrs->allowed_formats = DEFAULT_ALLOWED_FORMATS;
    glyrs->useragent = DEFAULT_USERAGENT;
    glyrs->force_utf8 = DEFAULT_FORCE_UTF8;
    glyrs->itemctr = 0;
}

/*-----------------------------------------------*/

void glyr_init_query(GlyrQuery * glyrs)
{
    set_query_on_defaults(glyrs);
}

/*-----------------------------------------------*/

void glyr_destroy_query(GlyrQuery * sets)
{
    if(sets)
    {
        for(gsize i = 0; i < 10; i++)
        {
            if(sets->info[i])
            {
                g_free((char*)sets->info[i]);
                sets->info[i] = NULL;
            }
        }

        /* Reset query so it can be used again */
        set_query_on_defaults(sets);
    }

}

/*-----------------------------------------------*/

GlyrMemCache * glyr_download(const char * url, GlyrQuery * s)
{
    return download_single(url,s,NULL);
}

/*-----------------------------------------------*/

void glyr_free_list(GlyrMemCache * head)
{
    if(head != NULL)
    {
        GlyrMemCache * next = head;
        GlyrMemCache * prev = head->prev;

        while(next != NULL)
        {
            GlyrMemCache * p = next;
            next = next->next;
            DL_free(p);
        }

        while(prev != NULL)
        {
            GlyrMemCache * p = prev;
            prev = prev->prev;
            DL_free(p);
        }
    }
}

/*-----------------------------------------------*/

void glyr_free_cache(GlyrMemCache * c)
{
    DL_free(c);
}

/*-----------------------------------------------*/

GlyrMemCache * glyr_new_cache(void)
{
    return DL_init();
}

/*-----------------------------------------------*/

// !! NOT THREADSAFE !! //
void glyr_init(void)
{
    /* Protect agains double initialization */
    if(is_initalized == FALSE)
    {
#if glyr_DEBUG
        /* Try to print informative output */
        signal(SIGSEGV, sig_handler);
#endif

        /* Init for threads */
        g_thread_init(NULL);

        if(curl_global_init(CURL_GLOBAL_ALL))
        {
            glyr_message(-1,NULL,"glyr: Fatal: libcurl failed to init\n");
        }

        /* Locale */
        if(setlocale (LC_ALL, "") == NULL)
        {
            glyr_message(-1,NULL,"glyr: Cannot set locale!\n");
        }

        /* Register plugins */
        register_fetcher_plugins();

        /* Init the smallest blacklist in the world :-) */
        blacklist_build();

        is_initalized = TRUE;
    }
}


/*-----------------------------------------------*/

// !! NOT THREADSAFE !! //
void glyr_cleanup(void)
{
    if(is_initalized == TRUE)
    {
        /* Curl no longer needed */
        curl_global_cleanup();

        /* Destroy all fetchers */
        unregister_fetcher_plugins();

        /* Kill it again */
        blacklist_destroy();

        is_initalized = FALSE;
    }
}



/*-----------------------------------------------*/

/* Sets parallel field depending on the get_type */
static void auto_detect_parallel(MetaDataFetcher * fetcher, GlyrQuery * query)
{
    if(query->parallel <= 0)
    {
        if(fetcher->default_parallel <= 0)
        {
            gint div = (int)(1.0/(CLAMP(query->qsratio,0.01,0.99) * 2));
            query->parallel = (div == 0) ? 3 : g_list_length(fetcher->provider) / div;
        }
        else
        {
            query->parallel = fetcher->default_parallel;
        }
    }
}

/*-----------------------------------------------*/

GlyrMemCache * glyr_get(GlyrQuery * settings, enum GLYR_ERROR * e, int * length)
{
    if(is_initalized == FALSE)
    {
        if(e != NULL)
        {
            *e = GLYRE_NO_INIT;
        }
        return NULL;
    }

    if(e) *e = GLYRE_OK;
    if(settings != NULL)
    {
        /* Print some user info, always useful */
        if(settings->artist != NULL)
        {
            glyr_message(2,settings,"- Artist   : %s\n",settings->artist);
        }
        if(settings->album != NULL)
        {
            glyr_message(2,settings,"- Album    : %s\n",settings->album);
        }
        if(settings->title != NULL)
        {
            glyr_message(2,settings,"- Title    : %s\n",settings->title);
        }

        GList * result = NULL;
        if(e) *e = GLYRE_UNKNOWN_GET;
        for(GList * elem = r_getFList(); elem; elem = elem->next)
        {
            MetaDataFetcher * item = elem->data;
            if(settings->type == item->type)
            {

                /* validate may be NULL, default to true */
                bool isValid = true;
                if(item->validate != NULL)
                {
                    isValid = item->validate(settings);
                }

                if(isValid)
                {
                    if(e) *e = GLYRE_OK;
                    glyr_message(2,settings,"- Type     : %s\n\n",item->name);

                    /* Lookup what we search for here: Images (url, or raw) or text */
                    settings->imagejob = !(item->full_data);

                    /* If ->parallel is <= 0, it gets autodetected */
                    auto_detect_parallel(item, settings);

                    /* Now start your engines, gentlemen */
                    result = start_engine(settings,item,e);
                    break;

                }
                else
                {
                    glyr_message(2,settings,C_R"Insufficient amount of data supplied for this fetcher.\n"C_);
                }
            }
        }

        /* Make this query reusable */
        settings->itemctr = 0;


        GlyrMemCache * head = NULL;

        /* free if empty */
        if(result != NULL)
        {
            /* Set the length */
            if(length != NULL)
            {
                *length = g_list_length(result);
            }

            /* link caches to each other */
            for(GList * elem = result; elem; elem = elem->next)
            {
                GlyrMemCache * item = elem->data;
                item->next = (elem->next) ? elem->next->data : NULL;
                item->prev = (elem->prev) ? elem->prev->data : NULL;
            }

            /* Finish. */
            if(g_list_first(result))
            {
                head = g_list_first(result)->data;
            }

            g_list_free(result);
            result = NULL;
        }
        return head;
    }
    if(e) *e = GLYRE_EMPTY_STRUCT;
    return NULL;
}

/*-----------------------------------------------*/

int glyr_write(GlyrMemCache * data, const char * path)
{
    int bytes = -1;
    if(path)
    {
        if(!strcasecmp(path,"null"))
        {
            bytes = 0;
        }
        else if(!strcasecmp(path,"stdout"))
        {
            bytes=fwrite(data->data,1,data->size,stdout);
            fputc('\n',stdout);
        }
        else if(!strcasecmp(path,"stderr"))
        {
            bytes=fwrite(data->data,1,data->size,stderr);
            fputc('\n',stderr);
        }
        else
        {
            FILE * fp = fopen(path,"w");
            if(fp)
            {
                if(data->data != NULL)
                {
                    bytes=fwrite(data->data,1,data->size,fp);
                }
                fclose(fp);
            }
            else
            {
                glyr_message(-1,NULL,"glyr_write: Unable to write to '%s'!\n",path);
            }
        }
    }
    return bytes;
}

/*-----------------------------------------------*/

static int glyr_set_info(GlyrQuery * s, int at, const char * arg)
{
    int result = GLYRE_OK;
    if(s && arg && at >= 0 && at < 10)
    {
        if(s->info[at] != NULL)
        {
            g_free((char*)s->info[at]);
        }

        s->info[at] = g_strdup(arg);
        switch(at)
        {
        case 0:
            s->artist = (gchar*)s->info[at];
            break;
        case 1:
            s->album = (gchar*)s->info[at];
            break;
        case 2:
            s->title = (gchar*)s->info[at];
            break;
        case 3:
            s->proxy = s->info[at];
            break;
        case 4:
            s->from = (gchar*)s->info[at];
            break;
        case 5:
            s->allowed_formats = (gchar*)s->info[at];
            break;
        case 6:
            s->useragent = (gchar*)s->info[at];
            break;
        case 7:
            s->lang = (gchar*)s->info[at];
            break;
        default:
            glyr_message(2,s,"Warning: wrong <at> for glyr_info_at!\n");
        }
    }
    else
    {
        result = GLYRE_BAD_VALUE;
    }
    return result;
}

/*-----------------------------------------------*/

const char * glyr_type_to_string(enum GLYR_DATA_TYPE type)
{
    switch(type)
    {
    case TYPE_COVER:
        return "cover";
    case TYPE_COVER_PRI:
        return "cover (frontside)";
    case TYPE_COVER_SEC:
        return "cover (backside or inlet)";
    case TYPE_LYRICS:
        return "songtext";
    case TYPE_PHOTOS:
        return "band photo";
    case TYPE_REVIEW:
        return "albumreview";
    case TYPE_AINFO:
        return "artistbio";
    case TYPE_SIMILIAR:
        return "similiar artist";
    case TYPE_SIMILIAR_SONG:
        return "similiar song";
    case TYPE_TRACK:
        return "trackname";
    case TYPE_ALBUMLIST:
        return "albumname";
    case TYPE_TAGS:
        return "some tag";
    case TYPE_TAG_ARTIST:
        return "artisttag";
    case TYPE_TAG_ALBUM:
        return "albumtag";
    case TYPE_TAG_TITLE:
        return "titletag";
    case TYPE_RELATION:
        return "relation";
    case TYPE_IMG_URL:
        return "Image URL";
    case TYPE_TXT_URL:
        return "HTML URL";
    case TYPE_NOIDEA:
    default:
        return "Unknown";
    }
}
/*-----------------------------------------------*/

void glyr_printitem(GlyrMemCache * cacheditem)
{
    // GlyMemcache members
    // dsrc = Exact link to the location where the data came from
    // size = size in bytes
    // type = Type of data
    // data = actual data
    // (error) - Don't use this. Only internal use
    g_printerr("FROM: <%s>\n",cacheditem->dsrc);
    g_printerr("PROV: %s\n",cacheditem->prov);
    g_printerr("SIZE: %d Bytes\n",(int)cacheditem->size);
    g_printerr("MSUM: ");
    MDPrintArr(cacheditem->md5sum);

    // Each cache identified it's data by a constant
    g_printerr("\nTYPE: ");
    if(cacheditem->type == TYPE_TRACK)
    {
        panic("[%02d:%02d] ",cacheditem->duration/60, cacheditem->duration%60);
    }
    g_printerr("%s",glyr_type_to_string(cacheditem->type));

    // Print the actual data.
    // This might have funny results if using cover/photos
    if(cacheditem->is_image == FALSE)
    {
        g_printerr("\nDATA: %s",cacheditem->data);
    }
    else
    {
        g_printerr("\nDATA: <not printable>");
        g_printerr("\nFRMT: %s",cacheditem->img_format);
    }
}

/* --------------------------------------------------------- */

GlyrFetcherInfo * glyr_get_plugin_info(void)
{
    return get_plugin_info();
}

/* --------------------------------------------------------- */

void glyr_free_plugin_info(GlyrFetcherInfo ** info)
{
    free_plugin_info(info);
}

/* --------------------------------------------------------- */
