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

// providers.
// Fill yours in here if you added a new one.
// The rest is done for you quite automagically.
sk_pair_t getwd_commands [] =
{
    {"cover" ,"..",NULL,true},
    {"lyrics","..",NULL,true},
    {"photos","..",NULL,true},
    {"books", "..",NULL,true},
    {NULL,   NULL, NULL,true}
};

/*-----------------------------------------------*/

void glyr_init_settings(glyr_settings_t * glyrs)
{
    glyrs->type = GET_UNSURE;

    glyrs->artist = NULL;
    glyrs->album  = NULL;
    glyrs->title  = NULL;

    glyrs->cover_min_size = 125;
    glyrs->cover_max_size = -1;

    glyrs->providers = NULL;

    glyrs->parallel  = 4;
    glyrs->save_path = ".";

    glyrs->AMAZON_LANG_ID = -1;
    glyrs->update = 0;
}

/*--------------------------------------------------------*/

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
        {
            n[i++] = *p++;
        }

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

static int has_next_word(const char * string, size_t length, size_t offset)
{
    if(string && offset <= length)
        return 1;

    return 0;
}

/*-----------------------------------------------*/

void glyr_register_group(sk_pair_t * providers, const char * groupname)
{
    int i = 0;

    while(providers[i].name)
    {
        if(providers[i].key == NULL && !strcasecmp(providers[i].name,groupname))
        {
            int back = i-1;
            while(back > 0 && providers[back].key != NULL)
            {
                providers[back].use = true;
                back--;
            }

            return;
        }
        i++;
    }
}
/*--------------------------------------------------------*/

static int sk_is_in(sk_pair_t * arr, const char * string)
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

            printf("%c '%s' ",(flag) ? ' ' : ',',arr[i].name);

            if(flag) flag = 0;
        }
        i++;
    }

    if(!flag) printf("?\n");
    return -1;
}

/*-----------------------------------------------*/

static sk_pair_t * glyr_get_provider_by_id(int ID)
{
	switch(ID)
	{
		case GET_COVER: return glyr_get_cover_providers();
		case GET_LYRIC: return glyr_get_lyric_providers();
		case GET_PHOTO: return glyr_get_photo_providers();
		case GET_BOOKS: return glyr_get_books_providers();
		default       : return NULL;
	}
}

/*-----------------------------------------------*/

void glyr_parse_from(const char * arg, glyr_settings_t * settings)
{
    if(settings && arg)
    {
        sk_pair_t * what_pair = glyr_get_provider_by_id(settings->type);
	settings->providers   = what_pair;

	if(what_pair)
	{
		char * c_arg = NULL;
		size_t length = strlen(arg);
		size_t offset = 0;

		if(arg[0] == '[' && arg[1] == '[')
		    offset = 2;

		while( (c_arg = next_word(arg," % ", length, &offset)) != NULL)
		{
		    if(!strcasecmp(c_arg,"all"))
		    {
			size_t y = 0;
			while(what_pair[y].name) what_pair[y++].use = true;
			free(c_arg);
			break;
		    }

		    if(!strcasecmp(c_arg,"safe"))
		    {
			glyr_register_group(what_pair, "safe");
			break;
		    }

		    if(!strcasecmp(c_arg,"unsafe"))
		    {
			glyr_register_group(what_pair, "unsafe");
			break;
		    }
		    size_t c_arg_len = strlen(c_arg);
		    if(c_arg_len > 1 && c_arg[c_arg_len-1] == ']' && c_arg[c_arg_len-2] == ']')
			c_arg[c_arg_len-2] = '\0';

		    sk_is_in( what_pair, c_arg);
		    free(c_arg);
		}
        }
    }
}

/*-----------------------------------------------*/

static void glyr_parse_get(const char * string, glyr_settings_t * glyrs)
{
    if(sk_is_in(getwd_commands,string) != -1)
    {
        if(!strcasecmp(string, "cover"))
        {
            glyrs->type = GET_COVER;
        }
        else if (!strcasecmp(string, "lyric"))
        {
            glyrs->type = GET_LYRIC;
        }
	else if (!strcasecmp(string, "photos"))
	{
	    glyrs->type = GET_PHOTO;
	}
	else if (!strcasecmp(string, "books"))
	{
	    glyrs->type = GET_BOOKS;
	}
	else
	{
	    glyrs->type = GET_UNSURE;
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

static void glyr_parse_minsize(const char * arg, glyr_settings_t * glyrs)
{
    if(glyrs->type == GET_COVER)
    {
        if(arg)
        {
            glyrs->cover_min_size = atoi(arg);
        }
    }
    else
    {
        printf("maxsize: This is only available for covers.\n");
    }
}

/*-----------------------------------------------*/

static void glyr_parse_maxsize(const char * arg, glyr_settings_t * glyrs)
{
    if(glyrs->type == GET_COVER)
    {
        if(arg)
        {
            glyrs->cover_max_size = atoi(arg);
        }
    }
    else
    {
        printf("maxsize: This is only available for covers.\n");
    }
}

/*-----------------------------------------------*/

static void glyr_parse_of(const char * arguments, glyr_settings_t * glyrs)
{
    if(arguments)
    {
        size_t a_len = strlen(arguments);
        size_t a_off = 0, type_counter = 0;

        if(arguments[0] == '[' && arguments[1] == '[')
            a_off = 2;

        char * curr_args = NULL;

        while( (curr_args = next_word(arguments," % ",a_len, &a_off)) != NULL)
        {
            switch(type_counter++)
            {
            case 0:
                //printf("Artist: %s\n",curr_args);
                glyrs->artist = curr_args;
                break;
            case 1:
                //printf("Album: %s\n",curr_args);
                glyrs->album = curr_args;
                break;
            case 2:
                //printf("Title: %s\n",curr_args);
                glyrs->title = curr_args;
                break;
            }
        }
    }
}

/*-----------------------------------------------*/

void glyr_elang(const char * descr, glyr_settings_t * glyrs)
{
    size_t offset = 0;
    size_t length = strlen(descr);
    char * string = (char*)descr;

    while(* string && *string == ' ')
    {
        string++;
    }

    char * word;

    while( (word = next_word(string, " ",length, &offset)) != NULL)
    {
        if(!strcasecmp(word,"and") || !strcasecmp(word,"with") || !strcasecmp(word,"to"))
        {
            continue;
        }
        else if(!strcasecmp(word,"get") && has_next_word(string,length,offset))
        {
            char * arguments = next_word(string," ", length, &offset);
            glyr_parse_get(arguments, glyrs);
            free(arguments);
        }
        else if(!strcasecmp(word,"update") )
        {
            glyrs->update = 1;
        }
        else if(!strcasecmp(word,"minsize") && has_next_word(string,length,offset) )
        {
            char * arg = next_word(string," ",length,&offset);
            glyr_parse_minsize(arg,glyrs);
            free(arg);
        }
        else if(!strcasecmp(word,"maxsize") && has_next_word(string,length,offset) )
        {
            char * arg = next_word(string," ",length,&offset);
            glyr_parse_maxsize(arg,glyrs);
            free(arg);
        }
        else if(!strcasecmp(word,"from") && has_next_word(string,length,offset) )
        {
            char * arguments = next_word(string,"]]", length, &offset);
            glyr_parse_from(arguments, glyrs);
            free(arguments);
        }
        else if (!strcasecmp(word,"of") && has_next_word(string,length,offset))
        {
            char * arguments = next_word(string,"]]",length, &offset);
            glyr_parse_of(arguments, glyrs);
            free(arguments);
        }
        else if(!strcasecmp(word,"write") && has_next_word(string,length,offset))
        {
            char * arg = next_word(string," ",length, &offset);
            glyr_parse_write(arg,glyrs);
            free(arg);
        }
        else
        {
            fprintf(stderr,"Unknown command or missing argument: '%s'..\n",word);
        }

        free(word);
        word = NULL;
    }
}

/*-----------------------------------------------*/

bool glyr_parse_commandline(int argc, char * const * argv, glyr_settings_t * glyrs)
{
    int c;
    while (true)
    {
        int option_index = 0;

        static struct option long_options[] =
        {
            {"get",      required_argument, 0, 'g'},
            {"from",     required_argument, 0, 'f'},
            {"of",       required_argument, 0, 'o'},
            {"minsize",  required_argument, 0, 'i'},
            {"maxsize",  required_argument, 0, 'a'},
            {"elang",    required_argument, 0, 'e'},
            {"write",    required_argument, 0, 'w'},
            {"update",   no_argument,       0, 'u'},
            {"help",     no_argument,       0, 'h'},
            {0,          0,                 0,  0 }
        };

        c = getopt_long(argc, argv, "e:g:f:o:i:a:w:uh",long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
        case 'o':
            glyr_parse_of(optarg,glyrs);
            break;

        case 'w':
            glyr_parse_write(optarg,glyrs);
            break;

        case 'g':
            glyr_parse_get(optarg,glyrs);
            break;

        case 'u':
            glyrs->update = 1;
            break;

        case 'i':
            glyr_parse_minsize(optarg,glyrs);
            break;

        case 'a':
            glyr_parse_maxsize(optarg,glyrs);
            break;

        case 'f':
            glyr_parse_from(optarg,glyrs);
            break;

        case 'e':
            glyr_elang(optarg,glyrs);
            break;

        case 'h':
            return false;
            break;
        }
    }

    if (optind < argc)
    {
        printf("Unused strings in cmdline: \n");
        while (optind < argc)
            printf("  '%s'\n", argv[optind++]);

        printf("\n");
    }
    return true;
}

/*-----------------------------------------------*/

void glyr_destroy_settings(glyr_settings_t * sets)
{
    if(sets)
    {
        if(sets->artist)
            free((char*)sets->artist);
        if(sets->album)
            free((char*)sets->album);
        if(sets->title)
            free((char*)sets->title);

        sets->type = GET_UNSURE;
        sets->cover_max_size = -1;
        sets->cover_min_size = -1;
        sets->providers = NULL;
        sets->parallel  = 4;
        sets->save_path = ".";
        sets->update = 0;
    }
}

char * glyr_path(glyr_settings_t * settings)
{
    if(settings && settings->save_path)
    {
        char * path = NULL;
        switch(settings->type)
        {
		case GET_COVER:
		{
		    char * esc_a = escape_slashes(settings->artist);
		    char * esc_b = escape_slashes(settings->album);
		    if(esc_a && esc_b)
		    {
			path = strdup_printf("%s/%s-%s.jpg",settings->save_path,esc_a,esc_b);
			free(esc_a);
			free(esc_b);
		    }
		    break;
		}
		case GET_LYRIC:
		{
		    char * esc_a = escape_slashes(settings->artist);
		    char * esc_t = escape_slashes(settings->title);
		    if(esc_a && esc_t)
		    {
			path = strdup_printf("%s/%s-%s.lyr",settings->save_path,esc_a,esc_t);
			free(esc_a);
			free(esc_t);
		    }
		    break;
		}
		case GET_PHOTO:
		{
			// Only dir is interesting.
			path = strdup(settings->save_path);
			break;
		}
		case GET_BOOKS:
		{
			char * esc_t = escape_slashes(settings->artist);
			if(esc_t)
			{
				path = strdup_printf("%s/%s.book",settings->save_path,esc_t);
				free(esc_t);
			}
		}
        }

        return path;
    }

    return NULL;
}

/*-----------------------------------------------*/

const char * glyr_get(glyr_settings_t * settings)
{
    if(settings->providers == NULL)
    {
	if( (settings->providers = glyr_get_provider_by_id(settings->type)) != NULL)
	{
		glyr_register_group((sk_pair_t*)settings->providers, "safe");
	}
	else
	{
		return NULL;
	}
    }

    char * result = NULL;
    char * path   = glyr_path(settings);

    if(path && (access(path,R_OK) || settings->update))
    {
        switch(settings->type)
        {
            case GET_COVER:
                result = get_cover(settings,path);
                break;
            case GET_LYRIC:
                result = get_lyrics(settings,path);
                break;
	    case GET_PHOTO:
		result = get_photos(settings,path);
		break;
	    case GET_BOOKS:
		result = get_books(settings,path);
		break;
            case GET_UNSURE:
                fprintf(stderr,C_R"ERROR:"C_" You did not specify a Get command!\n");
                break;
            default:
                fprintf(stderr,C_R"ERROR:"C_" Unknown GET Type..\n");
        }
        free(path);
    }
    else if(path)
    {
	fprintf(stdout,"%s\n",path);
    }
    return result;

}

/*-----------------------------------------------*/
