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

#include "../glyr_config.h"

// prot
static void glyr_parse_write(const char * arg, glyr_settings_t * glyrs);
static void glyr_parse_from(const char * arg, glyr_settings_t * settings);
static void glyr_parse_of(const char * arguments, glyr_settings_t * glyrs);
static void glyr_set_info(glyr_settings_t * s, int at, const char * arg);

// Fill yours in here if you added a new one.
// The rest is done for you quite automagically.
plugin_t getwd_commands [] =
{
    {"cover" ,"c", (char*)GET_COVER,false},
    {"lyrics","l", (char*)GET_LYRIC,false},
    {"photos","p", (char*)GET_PHOTO,false},
    {"books", "b", (char*)GET_BOOKS,false},
    {NULL,   NULL, NULL,  42}
};


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
			case GLYR_OPT_TYPE     : s->type = va_arg(param,int);              break;
			case GLYR_OPT_ARTIST   : glyr_set_info(s,0,va_arg(param,char*));   break;
			case GLYR_OPT_ALBUM    : glyr_set_info(s,1,va_arg(param,char*));   break;
			case GLYR_OPT_TITLE    : glyr_set_info(s,2,va_arg(param,char*));   break;
			case GLYR_OPT_CMAXSIZE : s->cover.min_size = va_arg(param,int);    break;	
			case GLYR_OPT_CMINSIZE : s->cover.max_size = va_arg(param,int);    break;
			case GLYR_OPT_UPDATE   : s->update    = va_arg(param,int);         break;
			case GLYR_OPT_PARALLEL : s->parallel  = va_arg(param,int);         break;
			case GLYR_OPT_AMAZON_ID: s->AMAZON_LANG_ID = va_arg(param,int);    break;
			case GLYR_OPT_NPHOTO   : s->photos.number = va_arg(param,int);     break;
			// -- cmdline convinience options -- //
			case GLYR_OPT_FROM     : glyr_parse_from(va_arg(param,char*),s);   break;
			case GLYR_OPT_PATH     : glyr_parse_write(va_arg(param,char *),s); break;
			case GLYR_OPT_OF       : glyr_parse_of(va_arg(param,char*),s);     break;
			default: result = GLYR_BAD_OPTION;
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
    // no idea what to do
    glyrs->type = GET_UNSURE;

    // ptr on ->info[]
    glyrs->artist = NULL;
    glyrs->album  = NULL;
    glyrs->title  = NULL;

    // merely sane defaults
    glyrs->cover.min_size = 125;
    glyrs->cover.max_size = -1;

    glyrs->photos.number = 5;
 
    // no list in ptrs
    memset(glyrs->info,0,sizeof(const char * ) * PTR_SPACE);

    // no provider at begin 
    glyrs->providers = NULL;

    // 4 parallel downloads
    glyrs->parallel  = 4;
    glyrs->redirects = DEFAULT_REDIRECTS;
    glyrs->timeout   = DEFAULT_TIMEOUT;

    // Save in working ddirectory
    glyrs->save_path = ".";

    // default server (.com)
    glyrs->AMAZON_LANG_ID = -1;
	
    // Do not update if already on disk
    glyrs->update = false;
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
        sets->cover.max_size = -1;
        sets->cover.min_size = -1;
        sets->providers = NULL;
        sets->parallel  = 4;
        sets->redirects = DEFAULT_REDIRECTS;
        sets->timeout   = DEFAULT_TIMEOUT;
        sets->save_path = ".";
        sets->update = 0;

	sets->artist = NULL;
	sets->album  = NULL;
	sets->title  = NULL;
	
	size_t i = 0;
	for(; i < PTR_SPACE;i++)
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
/* End of from outerspace visible methods.       */
/*-----------------------------------------------*/

static void glyr_set_info(glyr_settings_t * s, int at, const char * arg)
{
	if(s && arg && at >= 0)
	{
		s->info[at] = strdup(arg);
		switch(at)
		{
			case 0: s->artist  = s->info[at]; break;
			case 1: s->album   = s->info[at]; break;
			case 2: 
		     	    switch(s->type)
			    {
			        case GET_COVER: s->cover.min_size = atoi(s->info[at]); break;
			        case GET_LYRIC: s->title = s->info[at];                break;   
			    }
			    break;
		        case 3: 
		     	    switch(s->type)
			    {
			        case GET_COVER: s->cover.max_size = atoi(s->info[at]); break;
			    }
			    break;
			case 4:
			    switch(s->type)
		            {
				case GET_COVER: s->AMAZON_LANG_ID = atoi(s->info[at]); break;
			    }
			    break;
		}
	}
}

/*-----------------------------------------------*/

// Get next word (seperated by delim)
static char * next_word(const char * string, const char * delim,  size_t length, size_t *off)
{
    if(string && *off < length)
    {
        char * p = (char*)string + *off;
        while(*p && *p == *delim) p++;

        char * w = strstr(p,delim);
        char * n = malloc( (w) ? w-p+1 : length+1);
        size_t i = 0;

        while(*p && p != w)
            n[i++] = *p++;

        n[i] = '\0';
        *off = p - string + strlen(delim);

        if(*n)
        {
            return n;
        }
        else
        {
            free(n);
            n=NULL;
            return NULL;
        }
    }

    return NULL;
}

/*-----------------------------------------------*/

static void glyr_register_group(plugin_t * providers, const char * groupname)
{
    int i = 0;

    while(providers[i].name)
    {
        if(providers[i].key == NULL && !strcasecmp(providers[i].name,groupname))
        {
            int back = i-1;
            while(back >= 0 && providers[back].key != NULL)
                providers[back--].use = true;

            return;
        }
        i++;
    }
}
/*--------------------------------------------------------*/

static int is_in_list(plugin_t * arr, const char * string)
{
    size_t i = 0;
    while(arr[i].name)
    {
        if(!strcasecmp(arr[i].name,string) || (arr[i].key && !strcasecmp(arr[i].key,string)))
        {
            arr[i].use = true;
            return i;
        }

        i++;
    }

    // nothing found
    // suggest a getter that might be relevant
    printf(": Unknown word: '%s'\n",string);

    i = 0;
    char flag = 1;
    while(arr[i].name != NULL)
    {
        if(levenshtein_strcmp(arr[i].name,string) < 3)
        {
            if(flag)
            {
                printf(": Did you mean ");
            }

            printf("%c '%s' ",(flag) ? ' ' : ',',arr[i].name);

            if(flag) flag = 0;
        }
        i++;
    }

    if(!flag) printf("?\n");
    return -1;
}

/*-----------------------------------------------*/

static plugin_t * glyr_get_provider_by_id(int ID)
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
    default       :
        return NULL;
    }
}

/*-----------------------------------------------*/

static void glyr_parse_from(const char * arg, glyr_settings_t * settings)
{
    if(settings && arg)
    {
        plugin_t * what_pair = glyr_get_provider_by_id(settings->type);
        settings->providers   = what_pair;

        if(what_pair)
        {
            char * c_arg = NULL;
            size_t length = strlen(arg);
            size_t offset = 0;

            while( (c_arg = next_word(arg," % ", length, &offset)) != NULL)
            {
                if(!strcasecmp(c_arg,"all"))
                {
                    size_t y = 0;
                    while(what_pair[y].name) 
                        what_pair[y++].use = true;

		    free(c_arg);
		    return;
                }
                else if(!strcasecmp(c_arg,"safe"))
                {
                    glyr_register_group(what_pair, "safe");
                }
                else if(!strcasecmp(c_arg,"unsafe"))
                {
                    glyr_register_group(what_pair, "unsafe");
                }
		else if(!strcasecmp(c_arg,"special"))
		{
		    glyr_register_group(what_pair,"special");
		}
		else
		{
                    is_in_list( what_pair, c_arg);
		}

		// Give the user at least a hint.
                free(c_arg);
            }
        }
    }
}


/*-----------------------------------------------*/

static void glyr_parse_write(const char * arg, glyr_settings_t * glyrs)
{
    if(arg && access(arg, W_OK) )
    {
        printf("Unable to open %s\n",arg);
        return;
    }

    glyrs->save_path = arg;
}

/*-----------------------------------------------*/

static void glyr_parse_of(const char * arguments, glyr_settings_t * glyrs)
{
    if(arguments)
    {
        size_t a_len = strlen(arguments);
        size_t a_off = 0, i = 0;

        char * curr_args = NULL;
        while(i < PTR_SPACE && (curr_args = next_word(arguments," % ",a_len, &a_off)))
        {
		glyr_set_info(glyrs,i++,curr_args);
		free(curr_args);
		curr_args = NULL;
        }
    }
}

const char * glyr_get(glyr_settings_t * settings)
{
    if(settings->providers == NULL)
    {
	// Default to 'safe' group
        if( (settings->providers = glyr_get_provider_by_id(settings->type)) != NULL)
        {
            glyr_register_group((plugin_t*)settings->providers, "safe");
        }
        else
        {
	    // exit
            return NULL;
        }
    }

    char * result = NULL;
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
        case GET_UNSURE:
            fprintf(stderr,C_R"ERROR:"C_" You did not specify a --get command!\n");
            break;
        default:
            fprintf(stderr,C_R"ERROR:"C_" Unknown GET Type..\n");
    }

    return result;
}
