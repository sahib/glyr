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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>

#include "stringop.h"
#include "core.h"

#include "glyr.h"

#include "cover.h"
#include "lyrics.h"
#include "photos.h"
#include "books.h"
#include "ainfo.h"
#include "similiar.h"
#include "review.h"

#include "config.h"

// prot
static int glyr_parse_from(const char * arg, GlyQuery * settings);
static int glyr_set_info(GlyQuery * s, int at, const char * arg);
static void glyr_register_group(GlyPlugin * providers, enum GLYR_GROUPS GIDmask, bool value);

// Fill yours in here if you added a new one.
// The rest is done for you quite automagically.
GlyPlugin getwd_commands [] =
{
    {"cover" ,  "c",  (char*)GET_COVER,    false, {NULL, NULL, NULL, false}, GRP_NONE},
    {"lyrics",  "l",  (char*)GET_LYRIC,    false, {NULL, NULL, NULL, false}, GRP_NONE},
    {"photos",  "p",  (char*)GET_PHOTO,    false, {NULL, NULL, NULL, false}, GRP_NONE},
    {"ainfo",   "a",  (char*)GET_AINFO,    false, {NULL, NULL, NULL, false}, GRP_NONE},
    {"similiar","s",  (char*)GET_SIMILIAR, false, {NULL, NULL, NULL, false}, GRP_NONE},
    {"review",  "r",  (char*)GET_REVIEW,   false, {NULL, NULL, NULL, false}, GRP_NONE},
#ifdef USE_BOOKS
    /* Books was developed for private use     */
    /* You can enable it by defining USE_BOOKS */
    {"books",   "b",  (char*)GET_BOOKS,    false, {NULL, NULL, NULL, false}, GRP_NONE},
#endif
    {NULL,   NULL, NULL,  42,                     {NULL, NULL, NULL, false}, GRP_NONE}
};

/*-----------------------------------------------*/

const char * Gly_version(void)
{
    return "Version "glyr_VERSION_MAJOR"."glyr_VERSION_MINOR" ("glyr_VERSION_NAME") of ["__DATE__"] compiled at ["__TIME__"]";
}

/*-----------------------------------------------*/
// _opt_
/*-----------------------------------------------*/

// Seperate method because va_arg struggles with function pointers
int GlyOpt_dlcallback(GlyQuery * settings, int (*dl_cb)(GlyMemCache *, GlyQuery *), void * userp)
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

int GlyOpt_type(GlyQuery * s, int type)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    if(type >= GET_COVER && type < GET_UNSURE)
    {
        s->type = ABS(type);
        return GLYRE_OK;
    }
    return GLYRE_BAD_VALUE;
}

/*-----------------------------------------------*/

int GlyOpt_artist(GlyQuery * s, char * artist)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->artist = artist;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

int GlyOpt_album(GlyQuery * s, char * album)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->album = album;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

int GlyOpt_title(GlyQuery * s, char * title)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->title = title;
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

int GlyOpt_cmaxsize(GlyQuery * s, int size)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    return size_set(&s->cover.max_size,size);
}

/*-----------------------------------------------*/

int GlyOpt_cminsize(GlyQuery * s, int size)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    return size_set(&s->cover.min_size,size);
}

/*-----------------------------------------------*/

int GlyOpt_parallel(GlyQuery * s, unsigned long val)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->parallel = (long)val;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

int GlyOpt_timeout(GlyQuery * s, unsigned long val)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->timeout = (long)val;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

int GlyOpt_redirects(GlyQuery * s, unsigned long val)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->redirects = (long)val;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

int GlyOpt_lang(GlyQuery * s, char * langcode)
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

int GlyOpt_number(GlyQuery * s, unsigned int num)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->number = num;
    return GLYRE_OK;

}

/*-----------------------------------------------*/

int GlyOpt_verbosity(GlyQuery * s, unsigned int level)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->verbosity = level;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

int GlyOpt_infoat(GlyQuery * s, int at, const char * value)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    return glyr_set_info(s,at,value);
}

/*-----------------------------------------------*/

int GlyOpt_from(GlyQuery * s, const char * from)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    return glyr_parse_from(from,s);
}

/*-----------------------------------------------*/

int GlyOpt_color(GlyQuery * s, bool iLikeColorInMyLife)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->color_output = iLikeColorInMyLife;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

int GlyOpt_plugmax(GlyQuery * s, int plugmax)
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

int GlyOpt_duplcheck(GlyQuery * s, bool duplcheck)
{	
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->duplcheck = duplcheck;
    return GLYRE_OK;
}

/*-----------------------------------------------*/

int GlyOpt_groupedDL(GlyQuery * s, bool groupedDL)
{
	if(s == NULL) return GLYRE_EMPTY_STRUCT;
	s->groupedDL = groupedDL;
	return GLYRE_OK;
}

/*-----------------------------------------------*/

int GlyOpt_download(GlyQuery * s, bool download)
{
    if(s == NULL) return GLYRE_EMPTY_STRUCT;
    s->download = download;
    return GLYRE_OK;
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

const char * Gly_groupname_by_id(int ID)
{
	return grp_id_to_name(ID);
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
    glyrs->providers = NULL;
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
    glyrs->itemctr = 0;
    glyrs->duplcheck = DEFAULT_DUPLCHECK;
    memset(glyrs->info,0,sizeof(const char * ) * PTR_SPACE);
}

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
        for(; i < PTR_SPACE; i++)
        {
            if(sets->info[i])
            {
                free((char*)sets->info[i]);
                sets->info[i] = NULL;
            }
        }

        if(sets->providers != NULL)
	{
            free(sets->providers);
	    sets->providers = NULL;
	}

	set_query_on_defaults(sets);
    }
}

/*-----------------------------------------------*/

GlyMemCache * Gly_download(const char * url, GlyQuery * s)
{
	return download_single(url,s,NULL);
}

/*-----------------------------------------------*/

void Gly_free_list(GlyCacheList * lst)
{
    DL_free_lst(lst);
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

GlyCacheList * Gly_get(GlyQuery * settings, int * e)
{
    if(e) *e = GLYRE_OK;
    if(!settings->providers)
    {
        GlyPlugin * p = Gly_get_provider_by_id(settings->type);
        if(p != NULL)
        {
            glyr_register_group(p,GRP_ALL,true);
            settings->providers = p;
        }
        else
        {
            if(e) *e = GLYRE_NO_PROVIDER;
            return NULL;
        }
    }

    if(curl_global_init(CURL_GLOBAL_ALL))
    {
        glyr_message(-1,NULL,stderr,"?? libcurl failed to init.\n");
    }

    GlyCacheList * result = NULL;
    switch(settings->type)
    {
    case GET_COVER:
        result = get_cover(settings);
        break;
    case GET_LYRIC:
        result = get_lyrics(settings);
        break;
    case GET_PHOTO:
        result = get_photos(settings);
        break;
    case GET_BOOKS:
        result = get_books(settings);
        break;
    case GET_AINFO:
        result = get_ainfo(settings);
        break;
    case GET_SIMILIAR:
        result = get_similiar(settings);
        break;
    case GET_REVIEW:
        result = get_review(settings);
        break;
    default:
        if(e) *e = GLYRE_UNKNOWN_GET;
    }

    
    curl_global_cleanup();
    settings->itemctr = 0;
    return result;
}

/*-----------------------------------------------*/

int Gly_write_binary_file(const char * path, GlyMemCache * data, const char * save_dir, const char * type, GlyQuery *s)
{
    int bytes = -1;
    if(path)
    {
        if(!strcasecmp(save_dir,"null"))
        {
            bytes = 0;
        }
        else if(!strcasecmp(save_dir,"stdout"))
        {
            bytes=fwrite(data->data,1,data->size,stdout);
            fputc('\n',stdout);
        }
        else if(!strcasecmp(save_dir,"stderr"))
        {
            bytes=fwrite(data->data,1,data->size,stderr);
            fputc('\n',stderr);
        }
        else
        {
            glyr_message(1,s,stdout,""C_G"*"C_" Writing %s to %s\n",type,path);
            FILE * fp = fopen(path,"wb");
            if(fp)
            {
                bytes=fwrite(data->data,1,data->size,fp);
                fclose(fp);
            }
            else
            {
                glyr_message(-1,NULL,stderr,"Unable to write to '%s'!\n",path);
            }
        }
    }
    return bytes;
}

/*-----------------------------------------------*/

GlyPlugin * Gly_get_provider_by_id(int ID)
{
    switch(ID)
    {
    case GET_COVER:
        return glyr_get_cover_providers();
    case GET_LYRIC:
        return glyr_get_lyric_providers();
    case GET_PHOTO:
        return glyr_get_photo_providers();
    case GET_BOOKS:
        return glyr_get_books_providers();
    case GET_AINFO:
        return glyr_get_ainfo_providers();
    case GET_SIMILIAR:
        return glyr_get_similiar_providers();
    case GET_REVIEW:
        return glyr_get_review_providers();
    case -1       :
        return copy_table(getwd_commands,sizeof(getwd_commands));
    default       :
        return NULL;
    }
}

/*-----------------------------------------------*/
/* End of from outerspace visible methods.       */
/*-----------------------------------------------*/

static int glyr_set_info(GlyQuery * s, int at, const char * arg)
{
    int result = GLYRE_OK;
    if(s && arg && at >= 0 && at < PTR_SPACE)
    {
        if(s->info[at] != NULL)
            free((char*)s->info[at]);

        s->info[at] = strdup(arg);
        switch(at)
        {
        case 0:
            s->artist  = (char*)s->info[at];
            break;
        case 1:
            s->album = (char*)s->info[at];
            break;
        case 2:
            s->title = (char*)s->info[at];
            break;
        }
    }
    else
    {
        result = GLYRE_BAD_VALUE;
    }
    return result;
}

/*-----------------------------------------------*/

static void glyr_register_group(GlyPlugin * providers, enum GLYR_GROUPS GIDmask, bool value)
{
    int i = 0;
    if(GIDmask == GRP_ALL) /* (Un)Register ALL */
    {
        while(providers[i].name)
        {
            if(providers[i].key)
            {
                providers[i].use = value;
            }
            i++;
        }
        return;
    }
    else /* Register a specific Group */
    {
	    while(providers[i].name)
	    {
		if(providers[i].gid & GIDmask)
		{
			puts(providers[i].name);
			providers[i].use = value;
		}
		i++;
	    }
     }
}

/*-----------------------------------------------*/

static int glyr_parse_from(const char * arg, GlyQuery * settings)
{
    int result = GLYRE_OK;
    if(settings && arg)
    {
        GlyPlugin * what_pair = Gly_get_provider_by_id(settings->type);
	if(settings->providers != NULL)
	{
		free(settings->providers);
	}
        settings->providers = what_pair;

        if(what_pair)
        {
            char * c_arg = NULL;
            size_t length = strlen(arg);
            size_t offset = 0;

            while( (c_arg = get_next_word(arg,DEFAULT_FROM_ARGUMENT_DELIM, &offset, length)) != NULL)
            {
                char * track = c_arg;
                bool value = true;
                if(*c_arg && *c_arg == '-')
                {
                    value = false;
                    c_arg++;
                }
                else if(*c_arg && *c_arg == '+')
                {
                    value = true;
                    c_arg++;
                }

                if(!strcasecmp(c_arg, grp_id_to_name(GRP_ALL)))
                {
                    glyr_register_group(what_pair,GRP_ALL,value);
                }
                else if(!strcasecmp(c_arg, grp_id_to_name(GRP_SAFE)))
                {
                    glyr_register_group(what_pair, GRP_SAFE,value);
                }
                else if(!strcasecmp(c_arg, grp_id_to_name(GRP_USFE)))
                {
                    glyr_register_group(what_pair, GRP_USFE,value);
                }
                else if(!strcasecmp(c_arg, grp_id_to_name(GRP_SPCL)))
                {
                    glyr_register_group(what_pair, GRP_SPCL,value);
                }
                else if(!strcasecmp(c_arg, grp_id_to_name(GRP_FAST)))
                {
                    glyr_register_group(what_pair,GRP_FAST,value);
                }
                else if(!strcasecmp(c_arg, grp_id_to_name(GRP_SLOW)))
                {
                    glyr_register_group(what_pair,GRP_SLOW,value);
                }
                else
                {
                    int i = 0;
                    bool found = false;
                    for(; what_pair[i].name; i++)
                    {
                        if(!strcasecmp(what_pair[i].name,c_arg) || (what_pair[i].key && !strcasecmp(what_pair[i].key,c_arg)))
                        {
                            what_pair[i].use = value;
                            found = true;
                        }
                    }
                    if(!found)
                    {
                        glyr_message(1,settings,stderr,C_R"*"C_" Unknown provider '%s'\n",c_arg);
                        result = GLYRE_BAD_VALUE;
                    }
                }

                // Give the user at least a hint.
                free(track);
            }
        }
    }
    return result;
}

/*-----------------------------------------------*/
