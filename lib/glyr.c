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

#include "config.h"

// prot
static int glyr_parse_from(const char * arg, glyr_settings_t * settings);
static int glyr_set_info(glyr_settings_t * s, int at, const char * arg);
static void glyr_register_group(plugin_t * providers, const char * groupname, bool value);

// Fill yours in here if you added a new one.
// The rest is done for you quite automagically.
plugin_t getwd_commands [] =
{
    {"cover" ,"c", (char*)GET_COVER, false},
    {"lyrics","l", (char*)GET_LYRIC, false},
    {"photos","p", (char*)GET_PHOTO, false},
    {"ainfo","a",  (char*)GET_AINFO,false},
#ifdef USE_BOOKS
    /* Books was developed for private use     */
    /* You can enable it by defining USE_BOOKS */
    {"books", "b", (char*)GET_BOOKS, false},
#endif
    {NULL,   NULL, NULL,  42}
};

/*-----------------------------------------------*/

const char * glyr_version(void)
{
    return "Version "glyr_VERSION_MAJOR"."glyr_VERSION_MINOR" ("glyr_VERSION_NAME") of ["__DATE__"] compiled at ["__TIME__"]";
}

/*-----------------------------------------------*/

// Seperate method because va_arg struggles with function pointers
void glyr_set_dl_callback(glyr_settings_t * settings, void (*dl_cb)(memCache_t *, glyr_settings_t *), void * userp)
{
    if(settings)
    {
        settings->callback.download     = dl_cb;
        settings->callback.user_pointer = userp;
    }
}

/*-----------------------------------------------*/

// glyr_setopt()
// return: a GLYRE_ERROR (int) giving info. about the exit status
// param;
//  s:      a ptr to a glyr_settings_t structure, which you should have passed to glyr_init_settings before
//  option: the option that shall be modified (see glyr.h for possible values)
//  param : a va_list of arguments, every $option might require a different type
//          refer to glyr.h to find out what type is needed.
//

/*-----------------------------------------------*/

int glyr_setopt(glyr_settings_t * s, int option, ...)
{
    va_list param;
    va_start(param,option);
    int result = GLYRE_OK;
    if(s)
    {
        switch(option)
        {
        case GLYRO_TYPE     :
        {
            int arg = va_arg(param,int);
            s->type = ABS(arg);
            break;
        }
        case GLYRO_ARTIST   :
        {
            result = glyr_set_info(s,0,va_arg(param,char*));
            break;
        }
        case GLYRO_ALBUM    :
        {
            result = glyr_set_info(s,1,va_arg(param,char*));
            break;
        }
        case GLYRO_TITLE    :
        {
            result = glyr_set_info(s,2,va_arg(param,char*));
            break;
        }
        case GLYRO_CMAXSIZE :
        {
            s->cover.min_size = va_arg(param,int);
            if(s->cover.min_size < -1)
            {
                s->cover.min_size = DEFAULT_CMINSIZE;
                result = GLYRE_BAD_VALUE;
            }
            break;
        }
        case GLYRO_CMINSIZE :
        {
            s->cover.max_size = va_arg(param,int);
            if(s->cover.max_size < -1)
            {
                s->cover.max_size = DEFAULT_CMAXSIZE;
                result = GLYRE_BAD_VALUE;
            }
            break;
        }
        case GLYRO_PARALLEL :
        {
            long arg = va_arg(param,long);
            s->parallel = ABS(arg);
            if(!s->parallel)
            {
                s->parallel = DEFAULT_PARALLEL;
                result =  GLYRE_BAD_VALUE;
            }
            break;
        }
        case GLYRO_REDIRECTS:
        {
            long arg = va_arg(param,long);
            s->redirects = ABS(arg);
            break;
        }
        case GLYRO_TIMEOUT  :
        {
            long arg = va_arg(param,long);
            s->timeout = ABS(arg);
            break;
        }
        case GLYRO_LANG:
        {
            int arg = va_arg(param,int);
            if(arg >= 0 && arg <= GLYRL_JP)
            {
                s->lang = arg;
            }
            else
            {
                result = GLYRE_BAD_VALUE;
            }
            break;
        }
        case GLYRO_NUMBER   :
        {
            long arg = va_arg(param,long);
            s->number = ABS(arg);
            break;
        }
        case GLYRO_OFFSET :
        {
            int arg = va_arg(param,int);
            arg = ABS(arg);
            if(arg >= s->number)
            {
                glyr_message(2,s,stderr,C_R"[]"C_" offset is equal or higher than number. Setting to 0.\n");
                result = GLYRE_BAD_VALUE;
                arg = 0;
            }
            s->offset = arg;
            break;
        }
        case GLYRO_VERBOSITY:
        {
            int arg = va_arg(param,int);
            s->verbosity = ABS(arg);
            break;
        }
        // -- cmdline convinience options -- //
        case GLYRO_INFO_AT  :
        {
            result = glyr_set_info(s,va_arg(param,int),va_arg(param,char*));
            break;
        }
        case GLYRO_FROM     :
        {
            result = glyr_parse_from(va_arg(param,char*),s);
            break;
        }
        case GLYRO_COLOR    :
        {
            s->color_output = va_arg(param,int);
        }
        case GLYRO_PLUGMAX  :
        {
            int arg = va_arg(param,int);
            if(!arg) arg = 1;
            s->plugmax = ABS(arg);
            break;
        }
        case GLYRO_DOWNLOAD:
        {
            s->download = (bool)va_arg(param,int);
            break;
        }
        default:
        {
            result = GLYRE_BAD_OPTION;
            glyr_message(1,s,stderr,"Unknown option in glyr_setopt(): '%s'",option);
            break;
        }
        }
    }
    else
    {
        result = GLYRE_EMPTY_STRUCT;
    }

    va_end(param);
    return result;
}

/*-----------------------------------------------*/

void glyr_init_settings(glyr_settings_t * glyrs)
{
    glyrs->type = GET_UNSURE;
    glyrs->artist = NULL;
    glyrs->album  = NULL;
    glyrs->title  = NULL;
    glyrs->providers = NULL;

    glyrs->cover.min_size = DEFAULT_CMINSIZE;
    glyrs->cover.max_size = DEFAULT_CMAXSIZE;

    glyrs->number = DEFAULT_NUMBER;
    glyrs->offset = DEFAULT_OFFSET;
    glyrs->parallel  = DEFAULT_PARALLEL;
    glyrs->redirects = DEFAULT_REDIRECTS;
    glyrs->timeout   = DEFAULT_TIMEOUT;
    glyrs->verbosity = DEFAULT_VERBOSITY;
    glyrs->lang = DEFAULT_LANG;
    glyrs->plugmax = DEFAULT_PLUGMAX;

    glyrs->color_output = PRT_COLOR;

    glyrs->download = DEFAULT_DOWNLOAD;

    glyrs->callback.download = NULL;
    glyrs->callback.user_pointer = NULL;

    memset(glyrs->info,0,sizeof(const char * ) * PTR_SPACE);
}

/*-----------------------------------------------*/

void glyr_destroy_settings(glyr_settings_t * sets)
{
    if(sets)
    {
        if(sets->providers)
            free(sets->providers);

        // reset settings
        sets->type = GET_UNSURE;
        sets->cover.max_size = DEFAULT_CMAXSIZE;
        sets->cover.min_size = DEFAULT_CMINSIZE;

        sets->providers = NULL;
        sets->parallel  = DEFAULT_PARALLEL;
        sets->redirects = DEFAULT_REDIRECTS;
        sets->timeout   = DEFAULT_TIMEOUT;
        sets->verbosity = DEFAULT_VERBOSITY;
        sets->color_output = PRT_COLOR;

        sets->number  = DEFAULT_NUMBER;
        sets->offset  = DEFAULT_OFFSET;
        sets->plugmax = DEFAULT_PLUGMAX;

        sets->artist = NULL;
        sets->album  = NULL;
        sets->title  = NULL;

        sets->callback.download = NULL;
        sets->callback.user_pointer = NULL;

        sets->download = DEFAULT_DOWNLOAD;

        sets->lang = DEFAULT_LANG;

        size_t i = 0;
        for(; i < PTR_SPACE; i++)
        {
            if(sets->info[i])
            {
                free((char*)sets->info[i]);
                sets->info[i] = NULL;
            }
        }
    }
}

/*-----------------------------------------------*/

void glyr_free_list(cache_list * lst)
{
    DL_free_lst(lst);
}

/*-----------------------------------------------*/

void glyr_free_cache(memCache_t * c)
{
    DL_free(c);
}

/*-----------------------------------------------*/

void glyr_add_to_list(cache_list * l, memCache_t * c)
{
    DL_add_to_list(l,c);
}

/*-----------------------------------------------*/

cache_list * glyr_get(glyr_settings_t * settings, int * e)
{
    if(e) *e = GLYRE_OK;
    if(!settings->providers)
    {
        plugin_t * p = glyr_get_provider_by_id(settings->type);
        if(p != NULL)
        {
            glyr_register_group(p,"all",true);
            settings->providers = p;
        }
        else
        {
            if(e) *e = GLYRE_NO_PROVIDER;
            return NULL;
        }
    }

    cache_list * result = NULL;
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
    default:
        if(e) *e = GLYRE_UNKNOWN_GET;
    }

    return result;
}

/*-----------------------------------------------*/
/* End of from outerspace visible methods.       */
/*-----------------------------------------------*/

static int glyr_set_info(glyr_settings_t * s, int at, const char * arg)
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
            s->artist  = s->info[at];
            break;
        case 1:
            s->album = s->info[at];
            break;
        case 2:
            s->title = s->info[at];
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

static void glyr_register_group(plugin_t * providers, const char * groupname, bool value)
{
    int i = 0;
    if(!strcasecmp(groupname,"all"))
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
    while(providers[i].name)
    {
        if(providers[i].key == NULL && !strcasecmp(providers[i].name,groupname))
        {
            int back = i-1;
            while(back >= 0 && providers[back].key != NULL)
                providers[back--].use = value;

            return;
        }
        i++;
    }
}

/*-----------------------------------------------*/

plugin_t * glyr_get_provider_by_id(int ID)
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
    case -1       :
        return copy_table(getwd_commands,sizeof(getwd_commands));
    default       :
        return NULL;
    }
}

/*-----------------------------------------------*/

static int glyr_parse_from(const char * arg, glyr_settings_t * settings)
{
    int result = GLYRE_OK;
    if(settings && arg)
    {
        plugin_t * what_pair = glyr_get_provider_by_id(settings->type);
        settings->providers  = what_pair;

        if(what_pair)
        {
            char * c_arg = NULL;
            size_t length = strlen(arg);
            size_t offset = 0;

            while( (c_arg = get_next_word(arg,FROM_ARGUMENT_DELIM, &offset, length)) != NULL)
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

                if(!strcasecmp(c_arg,"all"))
                {
                    glyr_register_group(what_pair,"all",value);
                }
                else if(!strcasecmp(c_arg,"safe"))
                {
                    glyr_register_group(what_pair, "safe",value);
                }
                else if(!strcasecmp(c_arg,"unsafe"))
                {
                    glyr_register_group(what_pair, "unsafe",value);
                }
                else if(!strcasecmp(c_arg,"special"))
                {
                    glyr_register_group(what_pair,"special",value);
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
                        glyr_message(1,settings,stderr,C_R"[]"C_" Unknown provider '%s'\n",c_arg);
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
