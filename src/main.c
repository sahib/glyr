#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>

#include "../lib/glyr.h"

// also includes glyr's stringlib
// you shouldnt do this yourself, but
// implementing functions twice is silly
// (in this case at least )
#include "../lib/stringop.h"

#include <dirent.h>
#include <sys/types.h>

bool update = false;

//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static void usage(void)
{
    printf("Help text is still missing - Sorry.\n");
    printf("Usage Examples: \n");
    printf("\n");
    exit(0);
}

//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static bool set_get_type(glyr_settings_t * s, const char * arg)
{
    bool result = true;

    if(!arg)
        return result;

    // get list of avaliable commands
    plugin_t * plist = glyr_get_provider_by_id(-1);
    if(plist)
    {
        int i = 0;
        for(; plist[i].name; i++)
        {
            if(!strcmp(arg,plist[i].name) || *arg == *plist[i].name)
            {
                glyr_setopt(s,GLYRO_TYPE,plist[i].color);
                free(plist);
                return result;
            }
        }

        result = false;

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
    return result;
}

//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static void search_similiar_providers(const char * providers, glyr_settings_t * s)
{
    plugin_t * plist = glyr_get_provider_by_id(s->type);
    if(plist)
    {

        size_t length = strlen(providers);
        size_t offset = 0;
        char * name = NULL;

        while( (name = get_next_word(providers,FROM_ARGUMENT_DELIM,&offset,length)))
        {
            int j;

            bool f = false;
            for(j = 0; plist[j].name; j++)
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

                        glyr_message(2,s,stderr,"%s "C_G"%s"C_" ",dym?"":" or",plist[j].name);

                        dym = false;
                    }
                }
                if(!dym) glyr_message(2,s,stderr,"?\n",name);
            }
            free(name);
            name=NULL;
        }

        glyr_message(2,s,stderr,"\nAvailable providers:\n");
        glyr_message(2,s,stderr,  "--------------------\n");

        int i = 0;
        while( plist[i].name )
        {
            if(plist[i].key == NULL)
            {
                glyr_message(2,s,stderr,"group "C_G"#%s\n"C_,plist[i].name);

                int j = i-1;
                while(j >= 0 && plist[j].key && plist[j].name)
                {
                    glyr_message(2,s,stderr,"  %s (short: %s)\n",plist[j].name,plist[j].key);
                    j--;
                }
            }
            i++;
        }
        glyr_message(2,s,stderr,"\n");

        free(plist);
    }
}

//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */


static void suggest_other_options(int m, int argc, char * const * argv, int at, struct option * long_options, glyr_settings_t * s)
{
    bool dym = false;

    char * argument = argv[at];
    while(*argument && *argument == '-')
        argument++;

    glyr_message(1,s,stderr,C_R"[]"C_" Unknown option or missing argument: %s\n",argv[at]);
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
    if(dym)
    {
        glyr_message(1,s,stderr,"?\n");
    }
    if(optind == argc-1)
    {
        glyr_message(1,s,stderr,"\n");
    }
}

//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static void print_version(void)
{
    glyr_message(-1,NULL,stdout, C_G"%s\n\n"C_,glyr_version());
    glyr_message(-1,NULL,stderr, C_"Email bugs to <sahib@online.de> or use the bugtracker\n"
                 C_"at https://github.com/sahib/glyr/issues - Thank you! \n");
    exit(0);
}

//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * parse_commandline_general(int argc, char * const * argv, glyr_settings_t * glyrs)
{
    int c;
    char * v = NULL;

    static struct option long_options[] =
    {
        {"from",      required_argument, 0, 'f'},
        {"write",     required_argument, 0, 'w'},
        {"parallel",  required_argument, 0, 'p'},
        {"redirects", required_argument, 0, 'r'},
        {"timeout",   required_argument, 0, 'm'},
        {"plugmax",   required_argument, 0, 'x'},
        {"verbosity", required_argument, 0, 'v'},
        {"update",    no_argument,       0, 'u'},
        {"help",      no_argument,       0, 'h'},
        {"version",   no_argument,       0, 'V'},
        {"color",     no_argument,       0, 'c'},
        {"nodownload",no_argument,       0, 'd'},
        // -- plugin specific -- //
        {"artist",    required_argument, 0, 'a'},
        {"album",     required_argument, 0, 'b'},
        {"title",     required_argument, 0, 't'},
        {"minsize",   required_argument, 0, 'i'},
        {"maxsize",   required_argument, 0, 'e'},
        {"number",    required_argument, 0, 'n'},
        {"offset",    required_argument, 0, 'o'},
        {"lang",      required_argument, 0, 'l'},
        {0,           0,                 0,  0 }
    };

    while (true)
    {
        int option_index = 0;
        c = getopt_long(argc, argv, "uVhcdf:w:p:r:m:x:v:a:b:t:i:e:n:o:l:",long_options, &option_index);

        // own error report
        opterr = 0;

        if (c == -1)
            break;

        switch (c)
        {
        case 'w':
        {
            v = strdup(optarg);
            if(strcasecmp(optarg,"stdout") && strcasecmp(optarg,"stderr") && strcasecmp(optarg,"null"))
            {
                DIR * dummy = opendir(v);
                if(dummy)
                {
                    if(v != NULL)
                    {
                        size_t len = strlen(v);
                        if(v[len-1] == '/')
                        {
                            v[len-1] = 0;
                        }
                    }
                    closedir(dummy);
                }
                else
                {
                    glyr_message(2,glyrs,stderr,C_R"[]"C_" '%s' is not a valid directory!\n\n",optarg);
                    exit(-1);
                }
            }
            break;
        }
        case 'u':
            update = true;
            break;

        case 'f':
            if(glyr_setopt(glyrs,GLYRO_FROM,optarg) != GLYRE_OK)
            {
                search_similiar_providers(optarg,glyrs);
            }
            break;

        case 'v':
            glyr_setopt(glyrs,GLYRO_VERBOSITY,atoi(optarg));
            break;

        case 'p':
            glyr_setopt(glyrs,GLYRO_PARALLEL,atoi(optarg));
            break;

        case 'r':
            glyr_setopt(glyrs,GLYRO_REDIRECTS,atoi(optarg));
            break;

        case 'm':
            glyr_setopt(glyrs,GLYRO_TIMEOUT,atoi(optarg));
            break;

        case 'x':
            glyr_setopt(glyrs,GLYRO_PLUGMAX,atoi(optarg));
            break;

        case 'o':
            glyr_setopt(glyrs,GLYRO_OFFSET,atoi(optarg));
            break;

        case 'V':
            print_version();
            break;

        case 'h':
            usage();
            break;
        case 'a':
            glyr_setopt(glyrs,GLYRO_ARTIST,optarg);
            break;
        case 'b':
            glyr_setopt(glyrs,GLYRO_ALBUM,optarg);
            break;
        case 't':
            glyr_setopt(glyrs,GLYRO_TITLE,optarg);
            break;
        case 'i':
            glyr_setopt(glyrs,GLYRO_CMINSIZE,atoi(optarg));
            break;
        case 'e':
            glyr_setopt(glyrs,GLYRO_CMAXSIZE,atoi(optarg));
            break;
        case 'n':
            glyr_setopt(glyrs,GLYRO_NUMBER,atoi(optarg));
            break;
        case 'd':
            glyr_setopt(glyrs,GLYRO_DOWNLOAD,false);
            break;
        case 's':
            glyr_setopt(glyrs,GLYRO_COLOR,1);
            break;
        case 'l':
            if(!strcasecmp(optarg,"us"))
                glyr_setopt(glyrs,GLYRO_LANG,GLYRL_DE);
            else  if(!strcasecmp(optarg,"uk"))
                glyr_setopt(glyrs,GLYRO_LANG,GLYRL_DE);
            else  if(!strcasecmp(optarg,"ca"))
                glyr_setopt(glyrs,GLYRO_LANG,GLYRL_DE);
            else  if(!strcasecmp(optarg,"fr"))
                glyr_setopt(glyrs,GLYRO_LANG,GLYRL_DE);
            else  if(!strcasecmp(optarg,"de"))
                glyr_setopt(glyrs,GLYRO_LANG,GLYRL_DE);
            else  if(!strcasecmp(optarg,"jp"))
                glyr_setopt(glyrs,GLYRO_LANG,GLYRL_DE);
            else
            {
                glyr_message(1,glyrs,stderr,"Unknown language: "C_G"#%s\n"C_,optarg);
                glyr_message(1,glyrs,stderr,"Possible values:\n");
                glyr_message(1,glyrs,stderr,"    us\n");
                glyr_message(1,glyrs,stderr,"    uk\n");
                glyr_message(1,glyrs,stderr,"    ca\n");
                glyr_message(1,glyrs,stderr,"    fr\n");
                glyr_message(1,glyrs,stderr,"    de\n");
                glyr_message(1,glyrs,stderr,"    jp\n");
                glyr_message(1,glyrs,stderr,"      \n");
            }
            break;
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
    return v;
}

//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static int write_to_file(const char * path, memCache_t * data, const char * save_dir, const char * type, glyr_settings_t *s)
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
            glyr_message(1,s,stdout,""C_R"=>"C_" Writing %s to %s\n",type,path);
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
    }
    return bytes;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_covers(glyr_settings_t * s, const char * save_dir, int i)
{
    return strdup_printf("%s/%s_%s_%d.img",save_dir,s->artist,s->album,i);
}

static void handle_covers(memCache_t * cache, glyr_settings_t * s)
{
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_lyrics(glyr_settings_t * s, const char * save_dir, int i)
{
    return strdup_printf("%s/%s_%s_%d.lyrics",save_dir,s->artist,s->title,i);
}

static void handle_lyrics(memCache_t * cache, glyr_settings_t * s)
{
    bool have_album = (s->album != NULL);
    int dots = glyr_message(2,s,stderr,"\n%s by %s%s%s\n",s->title,s->artist,have_album ? " from ":"",have_album ? s->album : "");
    int d = 0;

    for(; d < dots; d++) glyr_message(1,s,stderr,"-");
    glyr_message(1,s,stdout,"\n%s\n\n",cache->data);
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_photos(glyr_settings_t * s, const char * save_dir, int i)
{
    return strdup_printf("%s/%s_photo#%d.img",save_dir,s->artist,i);
}

static void handle_photos(memCache_t* cache, glyr_settings_t * s)
{
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_ainfo(glyr_settings_t * s, const char * save_dir, int i)
{
    return strdup_printf("%s_artist_%d.descr\n",s->artist,i);
}

static void handle_ainfo(memCache_t * cache, glyr_settings_t *s)
{
    glyr_message(2,s,stdout,"\n%s\n",cache->data);
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_books(glyr_settings_t *s, const char * save_dir, int i)
{
    return strdup_printf("%s/%s.book_%d",save_dir,s->artist,i);
}

static void handle_books(memCache_t * cache, glyr_settings_t * s)
{
    glyr_message(1,s,stderr,"%s\n",cache->data);
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

char * get_path_by_type(glyr_settings_t * s, const char * sd, int iter)
{
    char * m_path = NULL;
    switch(s->type)
    {
    case GET_COVER:
        m_path = path_covers(s,sd,iter);
        break;
    case GET_LYRIC:
        m_path = path_lyrics(s,sd,iter);
        break;
    case GET_PHOTO:
        m_path = path_photos(s,sd,iter);
        break;
    case GET_BOOKS:
        m_path = path_books(s,sd,iter);
        break;
    case GET_AINFO:
        m_path = path_ainfo(s,sd,iter);
        break;
    }
    return m_path;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static void cb(memCache_t * c, glyr_settings_t * s)
{
    // This is just to demonstrate the callback option.
    // Put anything in here that you want to be executed when
    // a cache is 'ready' (i.e. ready for return)
    // See the glyr_set_dl_callback for more info
    // a custom pointer is in s->user_pointer
}

int main(int argc, char * argv[])
{
    int result = 0;

    if(argc >= 3)
    {
        // glyr's control struct
        glyr_settings_t my_settings;

        // Init to the default settings
        glyr_init_settings( &my_settings);

        // Set the type..
        if(!set_get_type(&my_settings, argv[1]))
        {
            glyr_destroy_settings( &my_settings );
            return EXIT_FAILURE;
        }

        char * save_dir = parse_commandline_general(argc-1, argv+1, &my_settings);

        // default to working dir
        const char * default_path = ".";
        if(!save_dir)
        {
            save_dir = (char*)default_path;
        }

        char *  m_path = NULL;
        char ** c_path = calloc(sizeof(char *), (my_settings.number+1));

        // Check if files do already exist
        bool file_exist = false;

        if(my_settings.type == GET_AINFO)
            my_settings.number++;

        size_t iter = 0;
        for(iter = 0; iter < my_settings.number; iter++)
        {

            m_path = get_path_by_type(&my_settings, save_dir, iter);
            if(m_path)
            {
                c_path[iter] = m_path;
                if(!update && (file_exist = !access(m_path,R_OK) ))
                {
                    break;
                }
            }
        }

        // Set (example) callback
        glyr_set_dl_callback(&my_settings, cb, NULL);

        size_t frec = my_settings.number;
        if(my_settings.type != GET_UNSURE)
        {
            if(!file_exist)
            {
                // Now download everything
                int get_error = GLYRE_OK;
                cache_list * my_list= glyr_get(&my_settings, &get_error);
                if(my_list)
                {
                    plugin_t * table_copy = glyr_get_provider_by_id(-1);
                    if(get_error == GLYRE_OK)
                    {
                        size_t i = 0;
                        for(i = 0; i < my_list->size && my_list->list[i]; i++)
                        {
                            switch(my_settings.type)
                            {
                            case GET_COVER:
                                handle_covers(my_list->list[i], &my_settings);
                                break;
                            case GET_LYRIC:
                                handle_lyrics(my_list->list[i], &my_settings);
                                break;
                            case GET_PHOTO:
                                handle_photos(my_list->list[i], &my_settings);
                                break;
                            case GET_BOOKS:
                                handle_books (my_list->list[i], &my_settings);
                                break;
                            case GET_AINFO:
                                handle_ainfo(my_list->list[i], &my_settings);
                                break;
                            }

                            if(i >= my_settings.number || c_path[i] == NULL)
                            {
                                c_path = realloc(c_path, sizeof(char * ) * (frec+2));
                                c_path[i] = get_path_by_type(&my_settings,save_dir,i);
                                frec++;
                            }

                            if(write_to_file(c_path[i],my_list->list[i],save_dir, table_copy[my_settings.type].name, &my_settings))
                                result = EXIT_FAILURE;
                        }
                    }

                    // Free all downloaded buffers
                    glyr_free_list(my_list);
                    free(table_copy);
                }
            }
            else
            {
                glyr_message(1,&my_settings,stderr,C_B"[]"C_" File(s) already exist. Use -u to update.\n");
            }

            if(c_path)
            {
                size_t i = 0;
                for(i = 0; i < frec; i++)
                {
                    if(c_path[i])
                    {
                        free(c_path[i]);
                        c_path[i] = NULL;
                    }
                }
                free(c_path);
                c_path = NULL;
            }

            // Clean memory alloc'd by settings
            glyr_destroy_settings( &my_settings);

            if(save_dir != default_path)
            {
                free(save_dir);
            }
        }
    }
    else if(argc >= 1 && !strcmp(argv[1],"-V"))
    {
        print_version();
    }
    else usage();

    // byebye
    return result;
}

//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */
