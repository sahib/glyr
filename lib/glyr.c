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
#ifdef USE_BOOKS
    /* Books was developed for private use     */
    /* You can enable it by defining USE_BOOKS */
    {"books", "b", (char*)GET_BOOKS, false},
#endif
    {"ainfo","a", (char*)GET_AINFO,false},
    {NULL,   NULL, NULL,  42}
};

const char * glyr_version(void)
{
    return "Version 0.1b of ["__DATE__"] compiled at ["__TIME__"]\n"
           "Email bugs to <sahib@online.de> or use the bugtracker\n"
           "at https://github.com/sahib/glyr/issues - Thank you! \n";
}

// glyr_setopt()
// return: a GLYR_ERROR (int) giving info. about the exit status
// param;
//  s:      a ptr to a glyr_settings_t structure, which you should have passed to glyr_init_settings before
//  option: the option that shall be modified (see glyr.h for possible values)
//  param : a va_list of arguments, every $option might require a different type
//          refer to glyr.h to find out what type is needed.
//

int glyr_setopt(glyr_settings_t * s, int option, ...)
{
    va_list param;
    va_start(param,option);
    int result = GLYR_OK;
    if(s)
    {
        switch(option)
        {
        case GLYR_OPT_TYPE     :
	{
            int arg = va_arg(param,int);
	    s->type = ABS(arg);
            break;
	}
        case GLYR_OPT_ARTIST   :
	{
            result = glyr_set_info(s,0,va_arg(param,char*));
            break;
	}
        case GLYR_OPT_ALBUM    :
	{
            result = glyr_set_info(s,1,va_arg(param,char*));
            break;
	}
        case GLYR_OPT_TITLE    :
	{
            result = glyr_set_info(s,2,va_arg(param,char*));
            break;
	}
        case GLYR_OPT_CMAXSIZE :
	{
            s->cover.min_size = va_arg(param,int);
	    if(s->cover.min_size < -1)
	    {
	 	s->cover.min_size = DEFAULT_CMINSIZE;
		result = GLYR_BAD_VALUE;
	    }
            break;
	}
        case GLYR_OPT_CMINSIZE :
	{
            s->cover.max_size = va_arg(param,int);
	    if(s->cover.max_size < -1) 
	    {
		s->cover.max_size = DEFAULT_CMAXSIZE;
		result = GLYR_BAD_VALUE;
	    }
            break;
	}
        case GLYR_OPT_UPDATE   :
	{
            s->update = va_arg(param,int);
            break;
	}
        case GLYR_OPT_PARALLEL :
	{
	    long arg = va_arg(param,long);
            s->parallel = ABS(arg);
	    if(!s->parallel) 
	    {
	    	s->parallel = DEFAULT_PARALLEL;
		result =  GLYR_BAD_VALUE;
	    }
            break;
	}
        case GLYR_OPT_REDIRECTS:
	{
	    long arg = va_arg(param,long);
            s->redirects = ABS(arg);
            break;
	}
        case GLYR_OPT_TIMEOUT  :
	{
	    long arg = va_arg(param,long);
            s->timeout = ABS(arg);
            break;
	}
        case GLYR_OPT_AMAZON_ID:
	{
            s->AMAZON_LANG_ID = va_arg(param,int);
	    if(s->AMAZON_LANG_ID < -1 || s->AMAZON_LANG_ID > 5) 
	    {
	    	s->AMAZON_LANG_ID = DEFAULT_AMAZON_ID;
		result = GLYR_BAD_VALUE;
	    }
            break;
	}
        case GLYR_OPT_NUMBER   :
	{
            long arg = va_arg(param,long);
	    s->number = ABS(arg);
            break;
	}
	case GLYR_OPT_OFFSET : 
	{
	    int arg = va_arg(param,int);
	    arg = ABS(arg);
	    if(arg >= s->number)
	    {
		glyr_message(2,s,stderr,C_R"[]"C_" offset is equal or higher than number. Setting to 0.\n");
		result = GLYR_BAD_VALUE;
		arg = 0;
	    }
	    s->offset = arg;
	    break;
	}
        case GLYR_OPT_VERBOSITY:
	{
            int arg = va_arg(param,int);
	    s->verbosity = ABS(arg);
            break;
	}
            // -- cmdline convinience options -- //
	case GLYR_OPT_INFO_AT  :
	{
	    result = glyr_set_info(s,va_arg(param,int),va_arg(param,char*)); 
	    break;
	}
        case GLYR_OPT_FROM     :
	{
            result = glyr_parse_from(va_arg(param,char*),s);
            break;
	}
	case GLYR_OPT_PLUGMAX  :
	{
	    int arg = va_arg(param,int);
	    if(!arg) arg = 1;
	    s->plugmax = ABS(arg);
	    break;
	}
        default:
	{
            result = GLYR_BAD_OPTION;
            glyr_message(1,s,stderr,"Unknown option in glyr_setopt(): '%s'",option);
            break;
	}
        }
    }
    else
    {
        result = GLYR_EMPTY_STRUCT;
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
    glyrs->AMAZON_LANG_ID = DEFAULT_AMAZON_ID;
    glyrs->update = DEFAULT_DO_UPDATE;
    glyrs->plugmax = DEFAULT_PLUGMAX;
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
        sets->update    = DEFAULT_DO_UPDATE;
        sets->verbosity = DEFAULT_VERBOSITY;

	sets->number  = DEFAULT_NUMBER;
	sets->offset  = DEFAULT_OFFSET;
	sets->plugmax = DEFAULT_PLUGMAX;

        sets->artist = NULL;
        sets->album  = NULL;
        sets->title  = NULL;

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

cache_list * glyr_get(glyr_settings_t * settings)
{
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
	    case GET_UNSURE:
		break;
    }

    return result;
}

/*-----------------------------------------------*/
/* End of from outerspace visible methods.       */
/*-----------------------------------------------*/

static int glyr_set_info(glyr_settings_t * s, int at, const char * arg)
{
    int result = GLYR_OK;
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
	result = GLYR_BAD_VALUE;
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
    int result = GLYR_OK;
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
			result = GLYR_BAD_VALUE;	
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
