#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>

#include "glyr.h"

// also includes glyr's stringlib
// you shouldnt do this yourself, but
// implementing functions twice is silly.
#include "stringop.h"

#include <dirent.h>
#include <sys/types.h>

// Here in the 'client' we can use as much global vars as we want.
// Pure pleasure :-)

const char * save_dir = NULL;

static void usage(void)
{
    printf("Help text is still missing - Sorry.\n");
    printf("Usage Examples: \n");
    printf("\t./glyr -g cover -o \"Fejd %% Eifur\" --from \"all\" --update\n");
    printf("\t./glyr -g lyric -o \"Fejd %% Eifur\" %% Drängen o kräkan\" --from \"safe\" --update\n");
    printf("\n");
    exit(0);
}

static void set_get_type(glyr_settings_t * s, const char * arg)
{
    if(!arg)
	return;

    // get list of avaliable commands
    plugin_t * plist = glyr_get_provider_by_id(-1);
    if(plist)
    {
    	int i = 0;
	for(; plist[i].name; i++)
	{
		if(!strcmp(arg,plist[i].name))
		{
			glyr_setopt(s,GLYR_OPT_TYPE,plist[i].color);
			free(plist);
			return;
		}
	}

	glyr_message(2,s,stderr,"Sorry, I don't know of a getter called '%s'...\n\n",arg);
	glyr_message(2,s,stderr,"Avaliable getters are: \n");
	glyr_message(2,s,stderr,"---------------------- \n");

	for(i = 0; plist[i].name; i++)
	{
		glyr_message(2,s,stderr,"%d) %s [%s]\n",i+1,plist[i].name,plist[i].key);
	}

	bool dym = false;
	for(i = 0; plist[i].name; i++)
	{
		if(levenshtein_strcmp(arg,plist[i].name) <= 4)
		{
			if(!dym) glyr_message(2,s,stderr,"\nPerhaps you've meant");
			glyr_message(2,s,stderr,"%s '%s'",dym?" or":" ",plist[i].name);
			dym=true;
		}
	}
	if(dym) glyr_message(2,s,stderr,"?\n");

	free(plist);
    }
}

static void search_similiar_providers(const char * providers, glyr_settings_t * s)
{
	plugin_t * plist = glyr_get_provider_by_id(s->type);
	if(plist)
	{
	
		size_t length = strlen(providers);
		size_t offset = 0;
		char * name = NULL;

		while( (name = get_next_word(providers,",",&offset,length)))
		{
			int j;

			bool f = false;
			for(j = 0; plist[j].name;j++)
			{
				if(!strcasecmp(plist[j].name,name) || (plist[j].key && !strcasecmp(plist[j].key,name)))
				{
					f = true;
					break;
				}
			}
			if(!f)
			{
				bool dym = true;
				for(j = 0; plist[j].name; j++)
				{
					if(levenshtein_strcmp(name,plist[j].name) < 3)
					{
						if(dym)
						{
						    glyr_message(2,s,stderr,C_G"[]"C_" Did you mean");
						}

						glyr_message(2,s,stderr,"%s "C_G"'%s'"C_" ",dym?"":" or",plist[j].name);
						
						dym = false;
					}
				}
				if(!dym) glyr_message(2,s,stderr,"?\n",name);
			}
			free(name);
			name=NULL;
		}
		free(plist);
	}
}

static void suggest_other_options(int m, int argc, char * const * argv, int at, struct option * long_options, glyr_settings_t * s)
{
    bool dym = false;

    char * argument = argv[at];
    while(*argument && *argument == '-') 
    	argument++;

    glyr_message(1,s,stderr,C_R"[]"C_" Unknown option '%s'\n",argv[at]);
    if(argument && *argument)
    {
	    int i = 0;
	    for(i = 0; i < m && long_options[i].name; i++)
	    {
		if(levenshtein_strcmp(argument,long_options[i].name) < 3 || *(argument) == long_options[i].val)
		{
		    if(!dym)
		    {
			glyr_message(1,s,stderr,C_G"[]"C_" Did you mean");
		    }

		    glyr_message(1,s,stderr,C_G"%s'--%s' (-%c) "C_,!dym?" ":"or",long_options[i].name,long_options[i].val);
		    dym=true;
		}
	    }
    }
    glyr_message(1,s,stderr,"?\n");

    if(optind == argc-1)
    {
	glyr_message(1,s,stderr,"\n");
    }
}


static void parse_commandline_general(int argc, char * const * argv, glyr_settings_t * glyrs)
{
    int c;

    static struct option long_options[] =
    {
        {"from",     required_argument, 0, 'f'},
        {"write",    required_argument, 0, 'w'},
        {"parallel", required_argument, 0, 'p'},
        {"redirects",required_argument, 0, 'r'},
        {"timeout",  required_argument, 0, 'm'},
        {"verbosity",required_argument, 0, 'v'},
        {"update",   no_argument,       0, 'u'},
        {"help",     no_argument,       0, 'h'},
        {"version",  no_argument,       0, 'V'},
	// -- plugin specific -- //
	{"artist",   required_argument, 0, 'a'},
	{"album",    required_argument, 0, 'b'},
	{"title",    required_argument, 0, 't'},
	{"minsize",  required_argument, 0, 'i'},
	{"maxsize",  required_argument, 0, 'x'},
	{"amazonid", required_argument, 0, 'z'},
	{"number",   required_argument, 0, 'n'},
        {"offset",   required_argument, 0, 'o'},
        {0,          0,                 0,  0 }
    };

    while (true)
    {
        int option_index = 0;
        c = getopt_long(argc, argv, "f:w:p:r:m:v:uhVa:b:t:i:x:z:n:o:",long_options, &option_index);

	// own error report
	opterr = 0;

        if (c == -1)
            break;

        switch (c)
        {
        case 'w':
        {
            DIR * dummy = opendir(optarg);
            if(dummy)
            {
                save_dir = optarg;
                closedir(dummy);
            }
            else
            {
                glyr_message(2,glyrs,stderr,C_R"[]"C_" '%s' is not a valid directory!\n\n",optarg);
                exit(-1);
            }

            break;
        }
        case 'u':
            glyr_setopt(glyrs,GLYR_OPT_UPDATE,true);
            break;

        case 'f':
            if(glyr_setopt(glyrs,GLYR_OPT_FROM,optarg) != GLYR_OK)
	    {
  	        search_similiar_providers(optarg,glyrs);
	    }
            break;

        case 'v':
            glyr_setopt(glyrs,GLYR_OPT_VERBOSITY,atoi(optarg));
            break;

        case 'p':
            glyr_setopt(glyrs,GLYR_OPT_PARALLEL,atoi(optarg));
            break;

        case 'r':
            glyr_setopt(glyrs,GLYR_OPT_REDIRECTS,atoi(optarg));
            break;

        case 'm':
            glyr_setopt(glyrs,GLYR_OPT_TIMEOUT,atoi(optarg));
            break;

	case 'o':
	    glyr_setopt(glyrs,GLYR_OPT_PHOTO_OFFSET,atoi(optarg));
	    break;

        case 'V':
            puts(glyr_version());
            exit(0);
            break;
        case 'h':
            usage();
            break;
	case 'a': 
		glyr_setopt(glyrs,GLYR_OPT_ARTIST,optarg);
		break;
	case 'b':
		glyr_setopt(glyrs,GLYR_OPT_ALBUM,optarg);
		break;
	case 't':
		glyr_setopt(glyrs,GLYR_OPT_TITLE,optarg);
		break;
	case 'i':
		glyr_setopt(glyrs,GLYR_OPT_CMINSIZE,atoi(optarg));
		break;
	case 'x': 
		glyr_setopt(glyrs,GLYR_OPT_CMAXSIZE,atoi(optarg));
		break;
	case 'z':
		glyr_setopt(glyrs,GLYR_OPT_AMAZON_ID,atoi(optarg));
		break;
	case 'n':
	{
		int n = atoi(optarg);
		switch(glyrs->type)
		{
			case GET_COVER: glyr_setopt(glyrs,GLYR_OPT_NCOVER,n); break;
			case GET_PHOTO: glyr_setopt(glyrs,GLYR_OPT_NPHOTO,n); break;
			default : glyr_message(2,glyrs,stderr,C_R"[]"C_" --number is only available for cover or photos\n");
		}
		break;
	}
	case '?':
		suggest_other_options(sizeof(long_options) / sizeof(struct option), argc, argv, optind-1, long_options,glyrs);
		break;
        }
    }

    if (optind < argc)
    {
        while (optind < argc)
        {
	    suggest_other_options(sizeof(long_options) / sizeof(struct option), argc, argv, optind, long_options, glyrs);
            optind++;
        }
    }
}

/* --------------------------------------------------------- */

static int write_to_file(const char * path, memCache_t * data)
{
    int bytes = -1;
    if(path)
    {
        FILE * fp = fopen(path,"w");
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
    return bytes;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_covers(glyr_settings_t * s, int i)
{
    return strdup_printf("%s/%s_%s_%d.img",save_dir,s->artist,s->album,i);
}

/* --------------------------------------------------------- */

static bool exist_cover(glyr_settings_t * s)
{
    bool result = true;
    int i = 0;
    for(; i < s->cover.number && result; i++)
    {
        char * path = path_covers(s,i);
        if(path)
        {
            if(access(path,R_OK))
                result = false;

            free(path);
        }
    }
    return result;
}

/* --------------------------------------------------------- */

static int handle_covers(memCache_t ** cache, glyr_settings_t * s)
{
    int result = 0;
    size_t i = 0;
    for(; cache[i]; i++)
    {
        char * path = path_covers(s,i);
        if(path)
        {
            glyr_message(1,s,stderr,"Writing image to %s\n",path);
            if(write_to_file(path,cache[i]) == -1)
                result = -1;

            free(path);
            path=NULL;
        }
    }
    return result;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_lyrics(glyr_settings_t * s)
{
    return strdup_printf("%s/%s_%s.lyrics",save_dir,s->artist,s->title);
}

/* --------------------------------------------------------- */

static bool exist_lyrics(glyr_settings_t *s)
{
    bool result = false;
    char * path = path_lyrics(s);
    if(path)
    {
        if(!access(path,R_OK))
            result = true;

        free(path);
    }
    return result;
}

/* --------------------------------------------------------- */

static int handle_lyrics(memCache_t * cache, glyr_settings_t * s)
{
    int result = 0;
    char * path = path_lyrics(s);
    if(path)
    {
        int dots = glyr_message(2,s,stderr,"\n%s by %s from %s\n",s->title,s->artist,s->album);
        int i = 0;

        for(; i < dots; i++) glyr_message(1,s,stderr,"-");
        glyr_message(1,s,stdout,"\n%s\n\n",cache->data);

        glyr_message(1,s,stderr,"Writing lyrics to '%s'\n",path);
        if(write_to_file(path,cache) == -1)
            result = -1;
        free(path);
    }
    return 0;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_photos(glyr_settings_t * s, int i)
{
    return strdup_printf("%s/%s_photo#%d.img",save_dir,s->artist,i);
}

/* --------------------------------------------------------- */

static bool exist_photos(glyr_settings_t * s)
{
    bool result = true;
    int i = 0;
    for(; i < s->photos.number && result; i++)
    {
        char * path = path_photos(s,i);
        if(path)
        {
            if(access(path,R_OK))
            {
                result = false;
            }
	    else
	    {
		puts(path);
            }

            free(path);
        }
    }
    return result;
}

/* --------------------------------------------------------- */

static int handle_photos(memCache_t** cache, glyr_settings_t * s)
{
    int result = 0;
    size_t i = s->photos.offset;
    for(; cache[i]; i++)
    {
        char * path = path_photos(s,i);
        if(path)
        {
            glyr_message(1,s,stderr,"Writing image to %s\n",path);
            if(write_to_file(path,cache[i]) == -1)
                result = -1;

            free(path);
            path=NULL;
        }
    }
    return result;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_adescr(glyr_settings_t * s)
{
	return strdup_printf("%s_artist.descr\n",s->artist);
}

static bool exist_adescr(glyr_settings_t * s)
{
	bool result = true;
	char * path = path_adescr(s);
	if(path)
	{
		if(access(path,R_OK))
		    result = false;	

		free(path);
	}
	return result;
}

static int handle_adescr(memCache_t ** cache, glyr_settings_t *s)
{
	int result = 0;
	char * path = path_adescr(s);
	if(path)
	{
		glyr_message(2,s,stdout,"Short\n-----\n%s\nLong\n----%s\n",cache[0]->data,cache[1]->data);

		free(path);
		path=NULL;
	}
	return result;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static bool exist_books(glyr_settings_t * s)
{
    return false;
}
static int handle_books(memCache_t * cache, glyr_settings_t * s)
{
    // nothing really..
    // books were only added because of personal use
    // ...
    return 0;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

int main(int argc, char * argv[])
{
    int result = 0;

    if(argc >= 2)
    {
        // glyr's control struct
        glyr_settings_t my_settings;

        // Init to the default settings
        glyr_init_settings( &my_settings);

        // Set the type..
        set_get_type(&my_settings, argv[1]);

        parse_commandline_general(argc-1, argv+1, &my_settings);

        // default to working dir
        if(!save_dir) save_dir = ".";

        bool file_exist = false;
        if(my_settings.update == false)
        {
            switch(my_settings.type)
            {
            case GET_COVER:
                file_exist = exist_cover(&my_settings);
                break;
            case GET_LYRIC:
                file_exist = exist_lyrics(&my_settings);
                break;
            case GET_PHOTO:
                file_exist = exist_photos(&my_settings);
                break;
            case GET_BOOKS:
                file_exist = exist_books(&my_settings);
                break;
	    case GET_ADESCR:
		file_exist = exist_adescr(&my_settings);
		break;
            }
        }

	if(my_settings.type != GET_UNSURE)
	{
		if(!file_exist)
		{
		    // Now execute...
		    memCache_t ** cache_lst = glyr_get(&my_settings);

		    if(cache_lst)
		    {
			switch(my_settings.type)
			{
			case GET_COVER:
			    result=handle_covers(cache_lst,   &my_settings);
			    break;
			case GET_LYRIC:
			    result=handle_lyrics(cache_lst[0],& my_settings);
			    break;
			case GET_PHOTO:
			    result=handle_photos(cache_lst   ,&my_settings);
			    break;
			case GET_BOOKS:
			    result=handle_books (cache_lst[0],&my_settings);
			    break;
			case GET_ADESCR:
			    result=handle_adescr(cache_lst,&my_settings);
			    break;
			}
			glyr_free_lst(&my_settings, cache_lst);
		    }
		    else
		    {
			result = 1;
		    }
		}
		else
		{
		    glyr_message(1,&my_settings,stderr,C_B"[]"C_" File(s) already exist. Use -u to update.\n");
		}
	}
        // Clean memory
        glyr_destroy_settings( &my_settings);
    }
    else
    {
        usage();
    }
    return result;
}
