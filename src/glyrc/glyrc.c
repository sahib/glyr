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

#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <glib.h>

#ifndef WIN32
  /* Backtrace*/
  #include <execinfo.h>
#endif

/* Somehow needed for g_vasprintf */
#include <glib/gprintf.h>

/* All you need from libglyr normally */
#include "../../lib/glyr.h"

/* Support for caching */
#include "../../lib/cache.h"

/* Compile information, you do not have this header */
#include "../../lib/config.h"

/* Silly autohelp feature */
#include "autohelp.h"

/* Globals */
const gchar * exec_on_call = NULL;
const gchar * from_string  = NULL;
gchar * write_to = NULL;

//* ------------------------------------------------------- */

static gint message(gint verbosity, GlyrQuery * s, FILE * stream, const gchar * fmt, ...)
{
    int written_bytes = -1;
    if(s || verbosity == -1)
    {
        if(verbosity == -1 || verbosity <= s->verbosity)
        {
            va_list param;
            char * tmp = NULL;
            if(stream && fmt)
            {
                va_start(param,fmt);
                written_bytes = g_vasprintf (&tmp,fmt,param);
                va_end(param);

                if(written_bytes != -1 && tmp)
                {
                    written_bytes = fprintf(stream,"%s%s",(verbosity>2)?"DEBUG: ":"",tmp);
                    g_free(tmp);
                    tmp = NULL;
                }
            }
        }
    }
    return written_bytes;
}

//* ------------------------------------------------------- */

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
        message(-1,NULL,stderr,"\nFATAL: libglyr stopped/crashed due to a %s of death.\n",g_strsignal(signal));
        message(-1,NULL,stderr,"       This is entirely the fault of the libglyr developers. Yes, we failed. Sorry. Now what to do:\n");
        message(-1,NULL,stderr,"       It would be just natural to blame us now, so just visit <https://github.com/sahib/glyr/issues>\n");
        message(-1,NULL,stderr,"       and throw hard words like 'backtrace', 'bug report' or even the '$(command I issued' at them).\n");
        message(-1,NULL,stderr,"       The libglyr developers will try to fix it as soon as possible so please stop pulling their hair.\n");
#ifndef WIN32
        message(-1,NULL,stderr,"\nA list of the last called functions follows, please add this to your report:\n");
        print_trace();
#endif
        message(-1,NULL,stderr,"\n(Thanks, and Sorry for any bad feelings.)\n\n");
        break;
    }
    exit(EXIT_FAILURE);
}


//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static void print_version(GlyrQuery * s)
{
    message(-1,s,stdout, "%s\n\n",glyr_version());
    message(-1,s,stderr, "This is still beta software, expect quite a lot bugs.\n");
    message(-1,s,stderr, "Email bugs to <sahib@online.de> or use the bugtracker\n"
            "at https://github.com/sahib/glyr/issues - Thank you! \n");

    exit(0);
}


//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

void help_short(GlyrQuery * s)
{
    message(-1,s,stderr,"Usage: glyrc [GETTER] (options)\n\nwhere [GETTER] must be one of:\n");
    GlyrFetcherInfo * info  = glyr_info_get();
    GlyrFetcherInfo * track = info;
    while(info != NULL)
    {
        g_print(" - %s\n", info->name);
        info = info->next;
    }
    glyr_info_free(track);

#define IN "    "
    message(-1,s,stderr,"\nGENERAL OPTIONS:\n"
            IN"-f --from             String: Providers from where to get metadata. Refer to glyrc --list for a full list\n"
            IN"-w --write            Path: Write metadata to the dir <d>, or filename <d>, special values stdout, stderr and null are supported\n"
            IN"-n --number           Integer: Download max. <n> items. Amount of actual downloaded items may be less.\n"
            IN"-t --lang             String: Language settings. Used by a few getters to deliever localized data. Given in ISO 639-1 codes like 'de'\n"
            IN"-o --only-lang        Only use providers that offer language specific data, this only applies for text items.\n"
            IN"-f --fuzzyness        Integer: Set treshold for level of Levenshtein algorithm.\n"
            IN"-q --qsratio          Float: How to weight quality/speed; 1.0 = full quality, 0.0 = full speed.\n"
            IN"-x --plugmax          Integer. Maximum number od download a plugin may deliever. Use to make results more vary.\n"
            IN"-v --verbosity        Integer. Set verbosity from 0 to 4. See --usage for details.\n"
            "\nNETWORK OPTIONS\n"
            IN"-p --parallel         Integer: Define the number of downloads that may be performed in parallel.\n"
            IN"-u --useragent        String: The useragent to use during HTTP requests\n"
            IN"-r --redirects        Integer. Define the number of redirects that are allowed.\n"
            IN"-m --timeout          Integer. Define the maximum number in seconds after which a download is cancelled.\n"
            IN"-k --proxy            String: Set the proxy to use in the form of [protocol://][user:pass@]yourproxy.domain[:port]\n"
            "\nPROVIDER SPECIFIC OPTIONS\n"
            IN"-d --download         Download Images.\n"
            IN"-D --skip-download    Don't download images, but return the URLs to them (act like a search engine)\n"
            IN"-a --artist           String: Artist name to search for\n"
            IN"-b --album            String: Album name to search for\n"
            IN"-t --title            String: Songname to search for\n"
            IN"-e --maxsize          Integer: (images only) The maximum size a cover may have.\n"
            IN"-i --minsize          Integer: (images only) The minimum size a cover may have.\n"
            IN"-F --formats          String: A semicolon seperated list of imageformats that are allowed. e.g.: \"png;jpeg\"\n"
            IN"-8 --force-utf8       Forces utf8 encoding for text items, invalid encodings get sorted out\n"
            "\nDATABASE OPTIONS\n"
            IN"-c --cache <folder>   Creates or opens an existing cache at <folder> and lookups data from there.\n"
            IN"-y --drop             Instead of searching for this element, the element is deleted from the database. Needs --cache.\n"
            IN"-g --list-db          List all items in the database (including the artist / album / title / type) - Needs --cache.\n"
            "\nMISC OPTIONS\n"
            IN"-L --list             List all fetchers and source providers for each and exit.\n"
            IN"-h --help             This text you unlucky wanderer are viewing.\n"
            IN"-V --version          Print the version string.\n"
            IN"-j --callback         Command: Set a bash command to be executed when a item is finished downloading;\n"
            IN"                      The special string <path> is expanded with the actual path to the data.\n"
            IN"\n\n"
            "With each item received you get a link to the original source, please refer to the individual terms of use,\n"
            "copying and distributing of this data might be not allowed.\n"
            "A more detailed version of this help can be found online: https://github.com/sahib/glyr/wiki/Commandline-arguments\n"
            );

    message(-1,s,stdout,"\nAUTHOR: (C) Christopher Pahl - 2011, <sahib@online.de>\n%s\n\n",glyr_version());
    exit(EXIT_FAILURE);
#undef IN
}

/* --------------------------------------------------------- */

static void visualize_from_options(void)
{
    g_print("# First line is the name of the fetcher you can use,\n"
            "# Second is the providername with the shortkey in []\n"
            "# Some unimportant information follows intented by '-'\n\n");

    GlyrFetcherInfo * info = glyr_info_get();
    if(info != NULL)
    {
        for(GlyrFetcherInfo * elem0 = info; elem0; elem0 = elem0->next)
        {
            g_print(" %s => %d\n",elem0->name,elem0->type);
            for(GlyrSourceInfo * elem1 = elem0->head; elem1; elem1 = elem1->next)
            {
                g_print("   # %s [%c]\n",elem1->name,elem1->key);
                g_print("     - Quality: %d\n",elem1->quality);
                g_print("     - Speed:   %d\n",elem1->speed);
                g_print("     - Type:    %d\n",elem1->type);
            }

            g_print(" + Requires: (%s%s%s)\n",
                    elem0->reqs & GLYR_REQUIRES_ARTIST ? "Artist " : "",
                    elem0->reqs & GLYR_REQUIRES_ALBUM  ? "Album "  : "",
                    elem0->reqs & GLYR_REQUIRES_TITLE  ? "Title"   : ""
                   );
            g_print(" + Optional: (%s%s%s)\n",
                    elem0->reqs & GLYR_OPTIONAL_ARTIST ? "Artist " : "",
                    elem0->reqs & GLYR_OPTIONAL_ALBUM  ? "Album "  : "",
                    elem0->reqs & GLYR_OPTIONAL_TITLE  ? "Title"   : ""
                   );

            g_print("\n///////////////////////////////\n");
        }

        g_print("\nFollowing providers work with all types:\n");
        g_print("  # local [l] (A local SQLite DB cache; see --cache)\n");
        g_print("\nThe string 'all' in --from enables all providers.\n");
        g_print("You can disable certain providers from this by prepending a '-':\n");
        g_print("  \"all;-lastfm\"\n");
        g_print("\n///////////////////////////////\n");
    }
    glyr_info_free(info);
}

/* --------------------------------------------------------- */

static void parse_commandline_general(int argc, char * const * argv, GlyrQuery * glyrs,
                                      gchar ** write_to, GlyrDatabase ** db,
                                      gboolean * delete_from_db, gboolean * iterate_over_db)
{
    static struct option long_options[] =
    {
        {"from",          required_argument, 0, 'f'},
        {"write",         required_argument, 0, 'w'},
        {"parallel",      required_argument, 0, 'p'},
        {"redirects",     required_argument, 0, 'r'},
        {"timeout",       required_argument, 0, 'm'},
        {"proxy",         required_argument, 0, 'k'},
        {"plugmax",       required_argument, 0, 'x'},
        {"useragent",     required_argument, 0, 'u'},
        {"verbosity",     required_argument, 0, 'v'},
        {"qsratio",       required_argument, 0, 'q'},
        {"formats",       required_argument, 0, 'F'},
        {"cache",         required_argument, 0, 'c'},
        {"drop",          no_argument,       0, 'y'},
        {"help",          no_argument,       0, 'h'},
        {"version",       no_argument,       0, 'V'},
        {"download",      no_argument,       0, 'd'},
        {"no-download",   no_argument,       0, 'D'},
        {"list",          no_argument,       0, 'L'},
        {"list-db",       no_argument,       0, 'g'},
        {"force-utf8",    no_argument,       0, '8'},
        // ---------- plugin specific ------------ //
        {"only-lang",     no_argument,       0, 'o'},
        {"artist",        required_argument, 0, 'a'},
        {"album",         required_argument, 0, 'b'},
        {"title",         required_argument, 0, 't'},
        {"minsize",       required_argument, 0, 'i'},
        {"maxsize",       required_argument, 0, 'e'},
        {"number",        required_argument, 0, 'n'},
        {"lang",          required_argument, 0, 'l'},
        {"fuzzyness",     required_argument, 0, 'z'},
        {"callback",	  required_argument, 0, 'j'},
        {0,               0,                 0, '0'}
    };

    while(TRUE)
    {
        gint c;
        gint option_index = 0;
        if((c = getopt_long_only(argc, argv, "f:w:p:r:m:x:u:v:q:c:yF:hVogdDLa:b:t:i:e:n:l:z:j:k:8",long_options, &option_index)) == -1)
        {
            break;
        }

        switch (c)
        {
            case 'w':
                {
                    gchar * dirname = g_path_get_dirname(optarg);
                    gsize opt_len = strlen(optarg);
                    if(g_ascii_strncasecmp(optarg,"stdout",opt_len) == 0 ||
                            g_ascii_strncasecmp(optarg,"stderr",opt_len) == 0 ||
                            g_ascii_strncasecmp(optarg,"null",  opt_len) == 0 ||
                            g_file_test(dirname,G_FILE_TEST_IS_DIR | G_FILE_TEST_EXISTS) == TRUE)
                    {
                        *write_to = optarg;
                        g_free(dirname);
                    }
                    else
                    {
                        g_free(dirname);
                        g_printerr("'%s' does not seem to be an valid directory!\n",optarg);
                        exit(-1);
                    }
                    break;
                }
            case 'f':
                glyr_opt_from(glyrs,optarg);
                from_string = optarg;
                break;
            case 'v':
                glyr_opt_verbosity(glyrs,atoi(optarg));
                break;
            case 'p':
                glyr_opt_parallel(glyrs,atoi(optarg));
                break;
            case 'r':
                glyr_opt_redirects(glyrs,atoi(optarg));
                break;
            case 'm':
                glyr_opt_timeout(glyrs,atoi(optarg));
                break;
            case 'u':
                glyr_opt_useragent(glyrs,optarg);
                break;
            case 'x':
                glyr_opt_plugmax(glyrs,atoi(optarg));
                break;
            case 'V':
                print_version(glyrs);
                break;
            case 'h':
                help_short(glyrs);
                break;
            case 'a':
                glyr_opt_artist(glyrs,optarg);
                break;
            case 'b':
                glyr_opt_album(glyrs,optarg);
                break;
            case 't':
                glyr_opt_title(glyrs,optarg);
                break;
            case 'i':
                glyr_opt_img_minsize(glyrs,atoi(optarg));
                break;
            case 'e':
                glyr_opt_img_maxsize(glyrs,atoi(optarg));
                break;
            case 'n':
                glyr_opt_number(glyrs,atoi(optarg));
                break;
            case 'o':
                glyr_opt_lang_aware_only(glyrs,true);
                break;
            case 'd':	
                glyr_opt_download(glyrs,true);
                break;
            case 'g':
                *iterate_over_db = TRUE;
                break;
            case 'D':
                glyr_opt_download(glyrs,false);
                break;
            case 'y':
                *delete_from_db = TRUE;
                break;
            case 'c':
                {
                    GlyrDatabase * new_db = glyr_db_init(optarg);
                    if(db == NULL)
                    {  
                        g_printerr("Unable to open or create a database at specified path.\n");
                        exit(EXIT_FAILURE);
                    }
                    *db = new_db;
                    glyr_opt_db_autowrite(glyrs,TRUE);
                    glyr_opt_lookup_db(glyrs,new_db); 
                }
                break;
            case 'l':
                glyr_opt_lang(glyrs,optarg);
                break;
            case 'L':
                visualize_from_options();
                exit(0);
                break;
            case 'z':
                glyr_opt_fuzzyness(glyrs,atoi(optarg));
                break;
            case 'j':
                exec_on_call = optarg;
                break;
            case 'k':
                glyr_opt_proxy(glyrs,optarg);
                break;
            case 'q':
                glyr_opt_qsratio(glyrs,atof(optarg));
                break;
            case 'F':
                glyr_opt_allowed_formats(glyrs,optarg);
                break;
            case '8':
                glyr_opt_force_utf8(glyrs,true);
                break;
            case '?':
                break;
            default:
                exit(0);
        }
    }
}

//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static void replace_char(gchar * string, gchar a, gchar b)
{
    if(string != NULL)
    {
        gsize str_len = strlen(string);
        for(gsize i = 0; i < str_len; i++)
        {
            if(string[i] == a)
            {
                string[i] = b;
            }
        }
    }
}

//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

gchar * correct_path(const char * path)
{
    gchar * result = NULL;
    if(path != NULL)
    {
        result = g_strdup(path);
        replace_char(result,'/','+');
        replace_char(result,' ','+');
    }
    return result;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

gchar * get_path_by_type(GlyrQuery * s, GlyrMemCache * c, const gchar * save_dir, gint num)
{
    gchar * artist = correct_path(s->artist);
    gchar * album  = correct_path(s->album);
    gchar * title  = correct_path(s->title);

    artist = (artist == NULL) ? "" : artist;
    album  = (album  == NULL) ? "" : album;
    title  = (title  == NULL) ? "" : title;

    gchar * type_string = (gchar*)glyr_get_type_to_string(s->type);
    gchar * specifier = g_strdup_printf("%s%s%s%s%s%s",
            artist,(*artist) ? "_" : "", 
            album ,(*album ) ? "_" : "", 
            title ,(*title ) ? "_" : "");

    gchar * result = g_strdup_printf("%s/%s%s_%d.%s",
            save_dir,
            specifier,
            type_string,
            num,
            (c->is_image) ? c->img_format : "txt");

    if(*artist) g_free(artist); 
    if(*album)  g_free(album);
    if(*title)  g_free(title);
    g_free(specifier);
    return result;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* -------------------------------------------------------- */

static GLYR_ERROR callback(GlyrMemCache * c, GlyrQuery * s)
{
    /* This is just to demonstrate the callback option.
     * Put anything in here that you want to be executed when
     * a cache is 'ready' (i.e. ready for return)
     * See the glyr_set_dl_callback for more info
     * a custom pointer is in s->callback.user_pointeur */
    int * current = s->callback.user_pointer;

    gchar * write_to_path = NULL;

    /* write out 'live' */
    if(write_to != NULL)
    {
        gsize write_len = strlen(write_to);
        if(g_ascii_strncasecmp(write_to,"stdout",write_len) == 0 ||
                g_ascii_strncasecmp(write_to,"stderr",write_len) == 0 ||
                g_ascii_strncasecmp(write_to,"null",  write_len) == 0)
        {
            glyr_cache_write(c,write_to);
        }
        else
        {
            if(g_file_test(write_to,G_FILE_TEST_IS_DIR) == TRUE)
            {
                write_to_path = get_path_by_type(s,c,write_to,*current);
            }
            else
            {
                write_to_path = g_strdup(write_to);
            }

            if(write_to_path != NULL)
            {
                if(glyr_cache_write(c,write_to_path) == -1)
                {
                    message(1,s,stderr,"(!!) glyrc: writing data to <%s> failed.\n",write_to_path);
                }
            }

            /* call the program if any specified */
            if(exec_on_call != NULL)
            {
                char ** path_splitv = g_strsplit(exec_on_call,"<path>",0);
                char * replace_path = g_strjoinv(write_to_path,path_splitv);
                g_strfreev(path_splitv);

                /* Call that command */
                int exitVal = system(replace_path);
                if(exitVal != EXIT_SUCCESS)
                {
                    message(3,s,stderr,"glyrc: cmd returned a value != EXIT_SUCCESS\n");
                }

                g_free(replace_path);
            }
        }
    }

    /* Text Represantation of this item */
    if(s->verbosity >= 1)
    {
        message(1,s,stderr,"\n///// ITEM #%d /////\n",(current) ? *current : -1);
        if(write_to_path != NULL)
        {
            message(1,s,stderr,"WRITE to '%s'\n",write_to_path);
            g_free(write_to_path);
        }
        glyr_cache_print(c);
        message(1,s,stderr,"\n////////////////////\n");
    }

    if(current != NULL)
    {
        *current += 1;
    }
    else
    {
        message(-1,NULL,stderr,"warning: Empty counterpointer!\n");
    }
    
    return GLYRE_OK;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* -------------------------------------------------------- */

GLYR_GET_TYPE get_type_from_string(gchar * string)
{
    GLYR_GET_TYPE result = GLYR_GET_UNSURE;
    if(g_ascii_strncasecmp(string,"none",4) == 0)
        return GLYR_GET_ANY;

    GlyrFetcherInfo * info = glyr_info_get();
    if(info != NULL)
    {
        for(GlyrFetcherInfo * elem = info; elem; elem = elem->next)
        {
            if(g_ascii_strncasecmp(elem->name,string,strlen(elem->name)) == 0)
            {
                result = elem->type;
                break;
            }
        }
    }
    else
    {
        g_printerr("Warning: Can't get type information. Probably a bug.\n");
    }
    glyr_info_free(info);
    return result;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* -------------------------------------------------------- */

static gint db_foreach_callback(GlyrQuery * q, GlyrMemCache * c, void * p)
{
    puts("\n////////////////////////////////////\n");
    g_print("%s :: %s :: %s :: %s\n",q->artist,q->album,q->title,glyr_get_type_to_string(q->type));
    glyr_cache_print(c);

    int * i = p;
    *i += 1;
    return 0;
}

/*----------------------*/

static void do_iterate_over_db(GlyrDatabase * db)
{
    gint db_item_counter = 0;
    glyr_db_foreach(db,db_foreach_callback,&db_item_counter);
    g_print("\n-----------------------------------\n");
    g_print("=> In total %d items in database.",db_item_counter);    
    g_print("\n-----------------------------------\n");
    g_print("\n");
}

/*----------------------*/

int main(int argc, char * argv[])
{
    /* Try to print informative output */
    signal(SIGSEGV, sig_handler);

    /* Assume success */
    gint result = EXIT_SUCCESS;

    /* Init. You _have_ to call this before making any calls */
    glyr_init();

    /* For every init, you should call glyr_cleanup,
     * Note: Both methods are NOT Threadsafe!
     */
    atexit(glyr_cleanup);

    if(argc >= 2 && argv[1][0] != '-')
    {
        /* Delete this item instead of searching for it */
        gboolean delete_from_db = FALSE;
    
        /* Shall we iterate over it? */
        gboolean iterate_over_db = FALSE;

        GlyrDatabase * db = NULL;

        /* The struct that control this beast */
        GlyrQuery my_query;

        /* set it on default values */
        glyr_query_init(&my_query);

        /* Default to a bit more verbose mode */
        glyr_opt_verbosity(&my_query,2);

        GLYR_GET_TYPE type = get_type_from_string(argv[1]);
        if(type == GLYR_GET_UNSURE)
        {
            g_print("glyr: \"%s\" is not a know getter. See `glyrc -L` for a list.\n",argv[1]);
            suggest_other_getter(&my_query,argv[1]);
            g_print("\n");
            exit(-1);
        }

        /* Set the type */
        my_query.type = type;

        /* Yes, user? */
        parse_commandline_general(argc-1, argv+1, &my_query,&write_to,&db, &delete_from_db, &iterate_over_db);

        if(write_to == NULL)
        {
            write_to = ".";
        }

        if(iterate_over_db)
        {
            do_iterate_over_db(db);
        }

        /* Set the callback - it will do all the actual work */
        gint item_counter = 0;
        glyr_opt_dlcallback(&my_query, callback, &item_counter);

        if(my_query.type != GLYR_GET_UNSURE)
        {
            if((iterate_over_db == FALSE && delete_from_db == FALSE) || db == NULL)
            {
                /* Now start searching! */
                int length = -1;
                GLYR_ERROR get_error = GLYRE_OK;
                GlyrMemCache * my_list = glyr_get(&my_query, &get_error, &length);

                if(my_query.verbosity >= 2)
                {
                    g_print("# => %d item(s) in total.\n",length);
                }

                if(my_list)
                {
                    /* Free all downloaded buffers */
                    glyr_free_list(my_list);

                }
                else if(get_error == GLYRE_NO_PROVIDER)
                {
                    g_print("glyr: --from \"%s\" does not contain any valid provider.\nSee `glyrc -L` for a list.\n",from_string);
                    suggest_other_provider(&my_query,(gchar*)from_string);
                }
                else if(get_error != GLYRE_OK)
                {
                    message(1,&my_query,stderr,"E: %s\n",glyr_strerror(get_error));
                }
            }
            else if(iterate_over_db == FALSE)
            {
                gint deleted = glyr_db_delete(db,&my_query);
                g_printerr("Deleted %d item%s from cache.\n",deleted, (deleted == 1) ? "" : "s");
            }
        }

        /* database */
        glyr_db_destroy(db);

        /* Clean memory alloc'd by settings */
        glyr_query_destroy( &my_query);
    }
    else if(argc >= 2 && (!strcmp(argv[1],"-V") || !strcmp(argv[1],"--list")))
    {
        print_version(NULL);
    }
    else if(argc >= 2 && (!strcmp(argv[1],"-L") || !strcmp(argv[1],"--list")))
    {
        visualize_from_options();
    }
    else
    {
        help_short(NULL);
    }

    return result;
}

//* --------------------------------------------------------- */
// ------------------End of program-------------------------- //
/* ---------------------------------------------------------- */
