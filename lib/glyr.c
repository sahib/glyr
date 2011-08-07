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

/* All you need is glove */
#include <glib.h>

#ifndef WIN32
  /* Backtrace*/
  #include <execinfo.h>
#endif

#include "glyr.h"
#include "core.h"
#include "register_plugins.h"
#include "blacklist.h"
#include "md5.h"

//* ------------------------------------------------------- */

gboolean is_initalized = FALSE;

//* ------------------------------------------------------- */

#ifndef WIN32
#define STACK_FRAME_SIZE 20
  /* Obtain a backtrace and print it to stdout. */
  static void print_trace(void)
  {
      void * array[STACK_FRAME_SIZE];
      char **bt_info_list;
      size_t size, i = 0;
  
      size = backtrace (array, STACK_FRAME_SIZE);
      bt_info_list = backtrace_symbols(array, size);
  
      for (i = 0; i < size; i++)
      {
          fprintf(stderr,"    [#%02u] %s\n",(int)i+1, bt_info_list[i]);
      }
  
      fprintf(stderr,"\n%zd calls in total are shown.\n", size);
      free(bt_info_list);
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
        glyr_message(-1,NULL,stderr,C_R"\nFATAL: "C_"libglyr stopped/crashed due to a %s of death.\n",g_strsignal(signal));
        glyr_message(-1,NULL,stderr,C_"       This is entirely the fault of the libglyr developers. Yes, we failed. Sorry. Now what to do:\n");
        glyr_message(-1,NULL,stderr,C_"       It would be just natural to blame us now, so just visit <https://github.com/sahib/glyr/issues>\n");
        glyr_message(-1,NULL,stderr,C_"       and throw hard words like 'backtrace', 'bug report' or even the '$(command I issued' at them).\n");
        glyr_message(-1,NULL,stderr,C_"       The libglyr developers will try to fix it as soon as possible so you stop pulling their hair.\n");
#ifndef WIN32
        glyr_message(-1,NULL,stderr,C_"\nA list of the last called functions follows, please add this to your report:\n");
        print_trace();
#endif
        glyr_message(-1,NULL,stderr,C_"\n(Thanks, and Sorry for any bad feelings.)\n\n");
        break;
    }
    exit(-1);
}

/*--------------------------------------------------------*/

const char * err_strings[] =
{
    "all okay",
    "bad option",
    "bad value for glyr_opt_*",
    "empty query",
    "No provider specified",
    "Unknown ID for getter",
    "Skipped cache",
    "Stopped by callback (post)",
    "Stopped by callback (pre)",
    "Library is not yet initialized, use glyr_init()",
    NULL
};

/*--------------------------------------------------------*/

static int glyr_set_info(GlyQuery * s, int at, const char * arg);
static void set_query_on_defaults(GlyQuery * glyrs);

/*--------------------------------------------------------*/
/*-------------------- OTHER -----------------------------*/
/*--------------------------------------------------------*/

GlyMemCache * glyr_copy_cache(GlyMemCache * source)
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
    return NULL;
}

/*-----------------------------------------------*/

void glyr_update_md5sum(GlyMemCache * cache)
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
enum GLYR_ERROR glyr_opt_dlcallback(GlyQuery * settings, DL_callback dl_cb, void * userp)
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

enum GLYR_ERROR glyr_opt_type(GlyQuery * s, enum GLYR_GET_TYPE type)
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

enum GLYR_ERROR glyr_opt_artist(GlyQuery * s, char * artist)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    glyr_set_info(s,0,artist);
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_album(GlyQuery * s, char * album)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    glyr_set_info(s,1,album);
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_title(GlyQuery * s, char * title)
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

enum GLYR_ERROR glyr_opt_cmaxsize(GlyQuery * s, int size)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    return size_set(&s->cover.max_size,size);
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_cminsize(GlyQuery * s, int size)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    return size_set(&s->cover.min_size,size);
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_parallel(GlyQuery * s, unsigned long val)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->parallel = (long)val;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_timeout(GlyQuery * s, unsigned long val)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->timeout = (long)val;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_redirects(GlyQuery * s, unsigned long val)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->redirects = (long)val;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_lang(GlyQuery * s, char * langcode)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    if(langcode != NULL)
    {
        s->lang = langcode;
        return GLYRE_OK;
    }
    return GLYRE_BAD_VALUE;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_number(GlyQuery * s, unsigned int num)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->number = num == 0 ? INT_MAX : num;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_verbosity(GlyQuery * s, unsigned int level)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->verbosity = level;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_from(GlyQuery * s, const char * from)
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

enum GLYR_ERROR glyr_opt_allowed_formats(GlyQuery * s, const char * formats)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    glyr_set_info(s,5,(formats==NULL) ? DEFAULT_ALLOWED_FORMATS : formats);
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_plugmax(GlyQuery * s, int plugmax)
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

enum GLYR_ERROR glyr_opt_qsratio(GlyQuery * s, float ratio)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->qsratio = MIN(MAX(ratio,0.0),1.0);
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_proxy(GlyQuery * s, const char * proxystring)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    glyr_set_info(s,3,proxystring);
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_fuzzyness(GlyQuery * s, int fuzz)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->fuzzyness = fuzz;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR glyr_opt_download(GlyQuery * s, bool download)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->download = download;
    return GLYRE_OK;
}

/*-----------------------------------------------*/
/*-----------------------------------------------*/
/*-----------------------------------------------*/

static void set_query_on_defaults(GlyQuery * glyrs)
{
    /* Initialize free pointer pool */
    memset(glyrs->info,0,sizeof(const char * ) * 10);

    glyrs->type = GET_UNSURE;
    glyrs->artist = NULL;
    glyrs->album  = NULL;
    glyrs->title  = NULL;
    glyrs->from   = NULL;
    glyrs->cover.min_size = DEFAULT_CMINSIZE;
    glyrs->cover.max_size = DEFAULT_CMAXSIZE;
    glyrs->number = DEFAULT_NUMBER;
    glyrs->parallel  = DEFAULT_PARALLEL;
    glyrs->redirects = DEFAULT_REDIRECTS;
    glyrs->timeout   = DEFAULT_TIMEOUT;
    glyrs->verbosity = DEFAULT_VERBOSITY;
    glyrs->lang = DEFAULT_LANG;
    glyrs->plugmax = DEFAULT_PLUGMAX;
    glyrs->download = DEFAULT_DOWNLOAD;
    glyrs->callback.download = NULL;
    glyrs->callback.user_pointer = NULL;
    glyrs->fuzzyness = DEFAULT_FUZZYNESS;
    glyrs->proxy = DEFAULT_PROXY;
    glyrs->qsratio = DEFAULT_QSRATIO;
    glyrs->allowed_formats = DEFAULT_ALLOWED_FORMATS;
    glyrs->itemctr = 0;
}

/*-----------------------------------------------*/

void glyr_init_query(GlyQuery * glyrs)
{
    set_query_on_defaults(glyrs);
}

/*-----------------------------------------------*/

void glyr_destroy_query(GlyQuery * sets)
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

GlyMemCache * glyr_download(const char * url, GlyQuery * s)
{
    return download_single(url,s,NULL);
}

/*-----------------------------------------------*/

void glyr_free_list(GlyMemCache * head)
{
    if(head != NULL)
    {
        GlyMemCache * next = head;
        GlyMemCache * prev = head->prev;

        while(next != NULL)
        {
            GlyMemCache * p = next;
            next = next->next;
            DL_free(p);
        }

        while(prev != NULL)
        {
            GlyMemCache * p = prev;
            prev = prev->prev;
            DL_free(p);
        }
    }
}

/*-----------------------------------------------*/

void glyr_free_cache(GlyMemCache * c)
{
    DL_free(c);
}

/*-----------------------------------------------*/

GlyMemCache * glyr_new_cache(void)
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
        /* Try to print informative output */
        signal(SIGSEGV, sig_handler);

        /* Init for threads */
        g_thread_init(NULL);

        if(curl_global_init(CURL_GLOBAL_ALL))
        {
            glyr_message(-1,NULL,stderr,"glyr: Fatal: libcurl failed to init\n");
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

GlyMemCache * glyr_get(GlyQuery * settings, enum GLYR_ERROR * e, int * length)
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
            glyr_message(2,settings,stderr,"- Artist : %s\n",settings->artist);
        }
        if(settings->album != NULL)
        {
            glyr_message(2,settings,stderr,"- Album  : %s\n",settings->album);
        }
        if(settings->title != NULL)
        {
            glyr_message(2,settings,stderr,"- Title  : %s\n",settings->title);
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
                    glyr_message(2,settings,stderr,"- Type   : %s\n",item->name);

                    /* Lookup what we search for here: Images (url, or raw) or text */
                    settings->imagejob = !item->full_data;

                    /* Now start your engines, gentlemen */
                    result = start_engine(settings,item);
                    break;

                }
                else
                {
                    glyr_message(2,settings,stderr,C_R"Insufficient amount of data supplied for this fetcher.\n"C_);
                }
            }
        }

        /* Make this query reusable */
        settings->itemctr = 0;

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
                GlyMemCache * item = elem->data;
                item->next = (elem->next) ? elem->next->data : NULL;
                item->prev = (elem->prev) ? elem->prev->data : NULL;
            }

            /* Finish. */
            GlyMemCache * head = NULL;
            if(g_list_first(result))
            {
                head = g_list_first(result)->data;
            }

            g_list_free(result);
            result = NULL;
            return head;
        }
    }
    if(e) *e = GLYRE_EMPTY_STRUCT;
    return NULL;
}

/*-----------------------------------------------*/

int glyr_write(GlyMemCache * data, const char * path)
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
            FILE * fp = fopen(path,"w" /* welcome back */);
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
                glyr_message(-1,NULL,stderr,"glyr_write: Unable to write to '%s'!\n",path);
            }
        }
    }
    return bytes;
}

/*-----------------------------------------------*/

static int glyr_set_info(GlyQuery * s, int at, const char * arg)
{
    int result = GLYRE_OK;
    if(s && arg && at >= 0 && at < 10)
    {
        if(s->info[at] != NULL)
        {
            g_free((char*)s->info[at]);
        }

        s->info[at] = strdup(arg);
        switch(at)
        {
        case 0:
            s->artist = (char*)s->info[at];
            break;
        case 1:
            s->album = (char*)s->info[at];
            break;
        case 2:
            s->title = (char*)s->info[at];
            break;
        case 3:
            s->proxy = s->info[at];
            break;
        case 4:
            s->from = (char*)s->info[at];
            break;
        case 5:
            s->allowed_formats = (char*)s->info[at];
            break;
        default:
            glyr_message(2,s,stderr,"Warning: wrong <at> for glyr_info_at!\n");
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

void glyr_printitem(GlyQuery *s, GlyMemCache * cacheditem)
{
	// GlyMemcache members
	// dsrc = Exact link to the location where the data came from
	// size = size in bytes
	// type = Type of data
	// data = actual data
	// (error) - Don't use this. Only internal use
	glyr_message(1,s,stderr,"FROM: <%s>\n",cacheditem->dsrc);
	glyr_message(1,s,stderr,"PROV: %s\n",cacheditem->prov);
	glyr_message(1,s,stderr,"SIZE: %d Bytes\n",(int)cacheditem->size);
	glyr_message(1,s,stderr,"MSUM: ");
	MDPrintArr(cacheditem->md5sum);

	// Each cache identified it's data by a constant
	glyr_message(1,s,stderr,"\nTYPE: ");
	if(cacheditem->type == TYPE_TRACK)
	{
		glyr_message(1,s,stderr,"[%02d:%02d] ",cacheditem->duration/60, cacheditem->duration%60);
	}
	glyr_message(1,s,stderr,"%s",glyr_type_to_string(cacheditem->type));

	// Print the actual data.
	// This might have funny results if using cover/photos
	if(!cacheditem->is_image)
	{
		glyr_message(1,s,stderr,"\nDATA:\n%s",cacheditem->data);
	}
	else
	{
		glyr_message(1,s,stderr,"\nDATA: <not printable>");
		glyr_message(1,s,stderr,"\nFRMT: %s",cacheditem->img_format);
	}
}

/* --------------------------------------------------------- */

GlyFetcherInfo * glyr_get_plugin_info(void)
{
	return get_plugin_info();
}

/* --------------------------------------------------------- */

void glyr_free_plugin_info(GlyFetcherInfo ** info)
{
	free_plugin_info(info);
}

/* --------------------------------------------------------- */
