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

#include <glib.h>

#ifndef WIN32
  /* Backtrace*/
  #include <execinfo.h>
#endif

#include "glyr.h"
#include "register_plugins.h"
#include "core.h"


//* ------------------------------------------------------- */

#ifndef WIN32

#define STACK_FRAME_SIZE 20
/* Obtain a backtrace and print it to stdout. */
void print_trace(void)
{
	void * array[STACK_FRAME_SIZE];
	char **bt_info_list;
	size_t size, i = 0;

	size = backtrace (array, STACK_FRAME_SIZE);
	bt_info_list = backtrace_symbols(array, size);

	for (i = 0; i < size; i++) {
	    fprintf(stderr,"    [#%02u] %s\n",(int)i+1, bt_info_list[i]);
	}

	fprintf(stderr,"\n%zd calls in total are shown.\n", size);
	free(bt_info_list);
}

#endif

//* ------------------------------------------------------- */

static void sig_handler(int signal)
{
        switch(signal) {
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

		g_on_error_stack_trace("glyr");
                break;
        }
        exit(-1);
}

/*--------------------------------------------------------*/

const char * err_strings[] =
{
	"all okay",
	"bad option",
	"bad value for option",
	"NULL pointer for struct",
	"No provider specified",
	"Unknown ID for getter",
	"Ignored cache",
	"Stopped by callback",
	NULL
};

/*--------------------------------------------------------*/

// prototypes
static int glyr_set_info(GlyQuery * s, int at, const char * arg);

/*--------------------------------------------------------*/
/*-------------------- OTHER -----------------------------*/
/*--------------------------------------------------------*/

GlyMemCache * Gly_copy_cache(GlyMemCache * source)
{
        return DL_copy(source);
}

/*--------------------------------------------------------*/

// return a descriptive string on error ID
const char * Gly_strerror(enum GLYR_ERROR ID)
{
	if(ID < (sizeof(err_strings)/sizeof(const char *)))
	{
		return err_strings[ID];
	}
	return NULL;
}

/*-----------------------------------------------*/

void Gly_update_md5sum(GlyMemCache * cache) 
{
        update_md5sum(cache);
}

/*-----------------------------------------------*/

const char * Gly_version(void)
{
        return "Version "glyr_VERSION_MAJOR"."glyr_VERSION_MINOR" ("glyr_VERSION_NAME") of ["__DATE__"] compiled at ["__TIME__"]";
}

/*-----------------------------------------------*/
// _opt_
/*-----------------------------------------------*/

// Seperate method because va_arg struggles with function pointers
enum GLYR_ERROR GlyOpt_dlcallback(GlyQuery * settings, DL_callback dl_cb, void * userp)
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

enum GLYR_ERROR GlyOpt_type(GlyQuery * s, enum GLYR_GET_TYPE type)
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

enum GLYR_ERROR GlyOpt_artist(GlyQuery * s, char * artist)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        glyr_set_info(s,0,artist);
        return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_album(GlyQuery * s, char * album)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        glyr_set_info(s,1,album);
        return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_title(GlyQuery * s, char * title)
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

enum GLYR_ERROR GlyOpt_cmaxsize(GlyQuery * s, int size)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        return size_set(&s->cover.max_size,size);
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_cminsize(GlyQuery * s, int size)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        return size_set(&s->cover.min_size,size);
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_parallel(GlyQuery * s, unsigned long val)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        s->parallel = (long)val;
        return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_timeout(GlyQuery * s, unsigned long val)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        s->timeout = (long)val;
        return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_redirects(GlyQuery * s, unsigned long val)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        s->redirects = (long)val;
        return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_lang(GlyQuery * s, char * langcode)
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

enum GLYR_ERROR GlyOpt_number(GlyQuery * s, unsigned int num)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        s->number = num == 0 ? INT_MAX : num;
        return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_verbosity(GlyQuery * s, unsigned int level)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        s->verbosity = level;
        return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_from(GlyQuery * s, const char * from)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
	if(from != NULL)
	{
        	glyr_set_info(s,7,from);
		return GLYRE_OK;
	}
	return GLYRE_BAD_VALUE;	
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_color(GlyQuery * s, bool iLikeColorInMyLife)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        s->color_output = iLikeColorInMyLife;
        return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_formats(GlyQuery * s, const char * formats)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        if(formats != NULL)
        {
                glyr_set_info(s,3,formats);
                return GLYRE_OK;
        }
        return GLYRE_BAD_VALUE;
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_plugmax(GlyQuery * s, int plugmax)
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

enum GLYR_ERROR GlyOpt_duplcheck(GlyQuery * s, bool duplcheck)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        s->duplcheck = duplcheck;
        return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_proxy(GlyQuery * s, const char * proxystring)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        glyr_set_info(s,6,proxystring);
        return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_fuzzyness(GlyQuery * s, int fuzz)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        s->fuzzyness = fuzz;
        return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_groupedDL(GlyQuery * s, bool groupedDL)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        s->groupedDL = groupedDL;
        return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_download(GlyQuery * s, bool download)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        s->download = download;
        return GLYRE_OK;
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_gtrans_target_lang(GlyQuery * s, const char * target)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        return glyr_set_info(s,4,target);
}

/*-----------------------------------------------*/

enum GLYR_ERROR GlyOpt_gtrans_source_lang(GlyQuery * s, const char * source)
{
        if(s == NULL) return GLYRE_EMPTY_STRUCT;
        return glyr_set_info(s,5,source);
}

/*-----------------------------------------------*/

GlyMemCache * Gly_clist_at(GlyCacheList * clist, int iter)
{
        if(clist && iter >= 0)
        {
                return clist->list[iter];
        }
        return NULL;
}

/*-----------------------------------------------*/
/*-----------------------------------------------*/
/*-----------------------------------------------*/

static void set_query_on_defaults(GlyQuery * glyrs)
{
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
        glyrs->color_output = PRT_COLOR;
        glyrs->download = DEFAULT_DOWNLOAD;
        glyrs->groupedDL = DEFAULT_GROUPEDL;
        glyrs->callback.download = NULL;
        glyrs->callback.user_pointer = NULL;
        glyrs->fuzzyness = DEFAULT_FUZZYNESS;
        glyrs->duplcheck = DEFAULT_DUPLCHECK;
        glyrs->formats = DEFAULT_FORMATS;
        glyrs->proxy = DEFAULT_PROXY;

        // gtrans - do no translation by default
        glyrs->gtrans.target = NULL;
        glyrs->gtrans.source = NULL;

        // Init freepointer pool
        memset(glyrs->info,0,sizeof(const char * ) * 10);
}

/*-----------------------------------------------*/

void Gly_init_query(GlyQuery * glyrs)
{
        set_query_on_defaults(glyrs);
}

/*-----------------------------------------------*/

void Gly_destroy_query(GlyQuery * sets)
{
        if(sets)
        {
                size_t i = 0;
                for(; i < 10; i++)
                {
                        if(sets->info[i])
                        {
                                free((char*)sets->info[i]);
                                sets->info[i] = NULL;
                        }
                }
	
		/* Reset query so it can be used again */
                set_query_on_defaults(sets);
        }

}

/*-----------------------------------------------*/

GlyMemCache * Gly_download(const char * url, GlyQuery * s)
{
        return download_single(url,s,NULL);
}

/*-----------------------------------------------*/

void Gly_free_list(GlyMemCache * head)
{
        if(head != NULL) {
                GlyMemCache * next = head;
                GlyMemCache * prev = head->prev;

                while(next != NULL) {
                        GlyMemCache * p = next;
                        next = next->next;
                        DL_free(p);
                }

                while(prev != NULL) {
                        GlyMemCache * p = prev;
                        prev = prev->prev;
                        DL_free(p);
                }
        }
}

/*-----------------------------------------------*/

void Gly_free_cache(GlyMemCache * c)
{
        DL_free(c);
}

/*-----------------------------------------------*/

GlyMemCache * Gly_new_cache(void)
{
        return DL_init();
}

/*-----------------------------------------------*/

void Gly_add_to_list(GlyCacheList * l, GlyMemCache * c)
{
        DL_add_to_list(l,c);

}

/*-----------------------------------------------*/

// !! NOT THREADSAFE !! //
void Gly_init(void)
{
	/* Protect agains double initialization */
        static bool already_init = false;
        if(already_init == false)
        {
        	signal(SIGSEGV, sig_handler);
                if(curl_global_init(CURL_GLOBAL_ALL))
                {
                        glyr_message(-1,NULL,stderr,"!! libcurl failed to init.\n");
                }
                already_init = true;

		/* Register plugins */
		register_fetcher_plugins();
        }
}


/*-----------------------------------------------*/

// !! NOT THREADSAFE !! //
void Gly_cleanup(void)
{
        static bool already_init = false;
        if(already_init == false)
        {
                curl_global_cleanup();
                already_init = true;
        }

	/* Destroy all fetchers */
	unregister_fetcher_plugins();
}



/*-----------------------------------------------*/

GlyMemCache * Gly_get(GlyQuery * settings, enum GLYR_ERROR * e, int * length)
{
        if(e) *e = GLYRE_OK;
	if(settings != NULL)
	{
		/* Print some user info, always useful */	
		if(settings->artist != NULL) {
			glyr_message(2,settings,stderr,"Artist: %s\n",settings->artist);
		}
		if(settings->album != NULL) {
			glyr_message(2,settings,stderr,"Album:  %s\n",settings->album);
		}
		if(settings->title != NULL) {
			glyr_message(2,settings,stderr,"Title:  %s\n",settings->title);
		}

		GList * result = NULL;
		if(e) *e = GLYRE_UNKNOWN_GET;
		for(GList * elem = r_getFList(); elem; elem = elem->next) { 
			MetaDataFetcher * item = elem->data;
			if(settings->type == item->type) {

				/* validate may be NULL, default to true */
				bool isValid = true;
				if(item->validate != NULL) {
				    isValid = item->validate(settings);   
				} 

				if(isValid) {
					if(e) *e = GLYRE_OK;
					glyr_message(2,settings,stderr,"Type:   %s\n",item->name);
					
					/* Now start your engines, gentlemen */
					result = start_engine(settings,item);
					break;

				} else {
					glyr_message(2,settings,stderr,C_R"Insufficient amount of data supplied for this fetcher.\n"C_);
				}
			}
		}

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
			if(g_list_first(result)) {
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

int Gly_write(GlyMemCache * data, const char * path)
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
				glyr_message(-1,NULL,stderr,"Gly_write: Unable to write to '%s'!\n",path);
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
			free((char*)s->info[at]);

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
				s->formats = (char*)s->info[at];
				break;
			case 4:
				s->gtrans.target = s->info[at];
				break;
			case 5:
				s->gtrans.source = s->info[at];
				break;
			case 6:
				s->proxy = s->info[at];
				break;
			case 7:
				s->from = (char*)s->info[at];
				break;
			default:
				glyr_message(2,s,stderr,"Warning: wrong $at for glyr_info_at!\n");
		}
	}
	else
	{
		result = GLYRE_BAD_VALUE;
	}
	return result;
}

/*-----------------------------------------------*/
