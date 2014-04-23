/***********************************************************
 * This file is part of glyr
 * + a commnadline tool and library to download various sort of music related metadata.
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
#include <glib/gstdio.h>

#ifdef __linux__
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

/* Update mechanism */
#include "update.h"

/* --callback and --write escape strings */
#include "escape.h"

/* cprint() */
#include "colorprint.h"

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

#define SET_IF_NULL(VAR, SET_TO) (VAR) = ((VAR) ? (VAR) : (SET_TO))
#define LOG_DOMAIN "Glyrc"

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

/* Shared data of main/callback */
typedef struct
{
    char * output_path;
    char * exec_on_call;
    bool as_one;
    bool append_format;

} callback_data_t;

//////////////////////////

#define INIT_CALLBACK_DATA \
{                          \
    .output_path  = NULL,  \
    .exec_on_call = NULL,  \
    .as_one       = false, \
    .append_format= false  \
}                          \
                                                     
//////////////////////////

static GlyrQuery * glob_lastQuery = NULL;

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void log_func (const gchar *log_domain,GLogLevelFlags log_level,const gchar *message, gpointer user_data)
{
    fputs (message,GLYR_OUTPUT);
}

////////////////////////////////////////////////////////////////

#ifdef __linux__
#define STACK_FRAME_SIZE 20

/* Obtain a backtrace and print it to stdout. */
static void print_trace (void)
{
    void * array[STACK_FRAME_SIZE];
    char ** bt_info_list;
    gsize size, it = 0;

    size = backtrace (array, STACK_FRAME_SIZE);
    bt_info_list = backtrace_symbols (array, size);

    for (it = 0; it < size; it++)
    {
        cvprint (DEFAULT,"    [#%02u] %s\n", (gint) it+1, bt_info_list[it]);
    }

    cvprint (DEFAULT,"\n%zd calls in total are shown.\n", size);
    g_free (bt_info_list);
}
#endif

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

static void sig_handler (int signal)
{
    switch (signal)
    {
    case SIGABRT :
    case SIGFPE  :
    case SIGSEGV : /* sigh */
        cvprint (RED,"\nFATAL: libglyr stopped/crashed due to a %s of death.\n",g_strsignal (signal) );
        cvprint (DEFAULT,"       This is entirely the fault of the libglyr developers. Yes, we failed. Sorry. Now what to do:\n");
        cvprint (DEFAULT,"       It would be just natural to blame us now, so just visit <https://github.com/sahib/glyr/issues>\n");
        cvprint (DEFAULT,"       and throw hard words like 'backtrace', 'bug report' or even the '$(command I issued' at them).\n");
        cvprint (DEFAULT,"       The libglyr developers will try to fix it as soon as possible so please stop pulling their hair.\n");
#ifdef __linux__
        cvprint (DEFAULT,"\nA list of the last called functions follows, please add this to your report:\n");
        print_trace();
#endif
        cvprint (DEFAULT,"\n(Thanks, and Sorry for any bad feelings.)\n\n");
        break;
    }
    exit (EXIT_FAILURE);
}

////////////////////////////////////////////////////////////////

static void init_signals (void)
{
    /* Only register for crashes */
    signal (SIGSEGV, sig_handler);
    signal (SIGABRT, sig_handler);
    signal (SIGFPE,  sig_handler);
}

////////////////////////////////////////////////////////////////

static void handle_cancel (int signo)
{
    cvprint (YELLOW,"Canceling..\n");
    if (glob_lastQuery != NULL)
    {
        glyr_signal_exit (glob_lastQuery);
    }
}

////////////////////////////////////////////////////////////////

static char * create_cache_directory (void)
{
    char * xdg_cache_dir = g_strdup(g_getenv("XDG_CACHE_HOME"));
    if(xdg_cache_dir == NULL) {
        xdg_cache_dir = (char *)g_getenv("HOME");
        if(xdg_cache_dir == NULL) {
            xdg_cache_dir = g_strdup("/tmp");
        } else {
            xdg_cache_dir = g_strdup_printf("%s%c.cache",
                xdg_cache_dir,
                G_DIR_SEPARATOR
            );
        }
    }

    char * full_path = g_strdup_printf("%s%cglyrc",
        xdg_cache_dir,
        G_DIR_SEPARATOR
    );

    if(!g_file_test(full_path, G_FILE_TEST_IS_DIR)) {
        g_printerr("<%s> does not exist yet. Creating...", full_path);
        if(g_mkdir_with_parents(full_path, 0777)  != 0) {
            g_printerr("Could not mkdir cache directory at %s\n", full_path);
            exit(EXIT_FAILURE);
        } else {
            g_printerr("done.\n");
        }
        
    }

    g_free(xdg_cache_dir);
    return full_path;
}

////////////////////////////////////////////////////////////////

static void init_cancel_handler (GlyrQuery * p_lastQuery)
{
    glob_lastQuery = p_lastQuery;
    signal (SIGINT, handle_cancel);
}

////////////////////////////////////////////////////////////////
// --------------------------------------------------------- //
///////////////////////////////////////////////////////////////

static void print_version (GlyrQuery * s)
{
    cprint (DEFAULT,-1,s, "%s\n\n",glyr_version() );

    cprint (DEFAULT,-1,s, "libglyr is licensed under the terms of the GPLv3.\n");
    cprint (DEFAULT,-1,s, "Please refer to the COPYING file you received with glyr.\n\n");
    cprint (DEFAULT,-1,s, "Email bugs to <sahib@online.de> or use the bugtracker\n"
            "at https://github.com/sahib/glyr/issues - Thank you! \n");

    exit (0);
}


////////////////////////////////////////////////////////////////
// --------------------------------------------------------- //
////////////////////////////////////////////////////////////////

void help_short (GlyrQuery * s)
{
    cprint (DEFAULT,-1,s,"Usage: glyrc [GETTER] (options)\n\nwhere [GETTER] must be one of:\n");
    GlyrFetcherInfo * info  = glyr_info_get();
    GlyrFetcherInfo * track = info;
    while (info != NULL)
    {
        cprint (DEFAULT,-1,NULL," - %s\n", info->name);
        info = info->next;
    }
    glyr_info_free (track);

#define IN "    "
    cprint (DEFAULT,-1,s,"\nGENERAL OPTIONS:\n"
            IN"-f --from                String: Providers from where to get metadata. Refer to glyrc --list for a full list\n"
            IN"-n --number              Integer: Download max. <n> items. Amount of actual downloaded items may be less.\n"
            IN"-l --lang                String: Language settings. Used by a few getters to deliever localized data. Given in ISO 639-1 codes like 'de'\n"
            IN"-o --only-lang           Only use providers that offer language specific data, this only applies for text items.\n"
            IN"-z --fuzzyness           Integer: Set threshold for level of Levenshtein algorithm.\n"
            IN"-q --qsratio             Float: How to weight quality/speed; 1.0 = full quality, 0.0 = full speed.\n"
            IN"-x --plugmax             Integer. Maximum number od download a plugin may deliever. Use to make results more vary.\n"
            IN"-v --verbosity           Integer. Set verbosity from 0 to 4. See online-README for details.\n"
            IN"-w --write               Path: Write metadata to the dir <d>, or filename <d>, special values stdout, stderr and null are supported\n"
            IN"                               Certain escapes are escaped inside the filename in the form :X:, where X may be one of:\n"
            IN"                                  * type     : The getter used in this query (e.g. cover)\n"
            IN"                                  * artist   : Artist - as passed by -a\n"
            IN"                                  * album    : Album  - as passed by -b\n"
            IN"                                  * title    : Title  - as passed by -t\n"
            IN"                                  * size     : Size in bytes\n"
            IN"                                  * format   : The image format or 'txt' for textitems\n"
            IN"                                  * source   : The source URL of this item\n"
            IN"                                  * prov     : The Provider of this item\n"
            IN"                                  * cksum    : a md5sum of the data\n"
            IN"                                  * rating   : (Usually 0)\n"
            IN"                                  * duration : (Only for tracks) Duration in seconds\n"
            IN"                                  * number   : An index incremented with each item (starting with 1)\n"
            IN"\n"
            IN"                               The default format is ':artist:_:album:_:title:_:type:_:number:.:format:'\n"
            IN"                               Strings containing '/' are replaced with '|' automatically,\n"
            IN"                               so --write ':artist:' for AC/DC will result in AC|DC\n"
            IN"\n"
            "\nNETWORK OPTIONS\n"
            IN"-p --parallel            Integer: Define the number of downloads that may be performed in parallel.\n"
            IN"-u --useragent           String: The useragent to use during HTTP requests\n"
            IN"-r --redirects           Integer. Define the number of redirects that are allowed.\n"
            IN"-m --timeout             Integer. Define the maximum number in seconds after which a download is cancelled.\n"
            IN"-k --proxy               String: Set the proxy to use in the form of [protocol://][user:pass@]yourproxy.domain[:port]\n"
            "\nPROVIDER SPECIFIC OPTIONS\n"
            IN"-d --download            Download Images.\n"
            IN"-D --skip-download       Don't download images, but return the URLs to them (act like a search engine)\n"
            IN"-a --artist              String: Artist name to search for\n"
            IN"-b --album               String: Album name to search for\n"
            IN"-t --title               String: Songname to search for\n"
            IN"-e --maxsize             Integer: (images only) The maximum size a cover may have.\n"
            IN"-i --minsize             Integer: (images only) The minimum size a cover may have.\n"
            IN"-F --formats             String: A semicolon separated list of imageformats that are allowed. e.g.: \"png;jpeg\"\n"
            IN"-8 --force-utf8          Forces utf8 encoding for text items, invalid encodings get sorted out\n"
            "\nMISC OPTIONS\n"
            IN"-L --list                List all fetchers and source providers for each and exit.\n"
            IN"-h --help                This text you unlucky wanderer are viewing.\n"
            IN"-V --version             Print the version string.\n"
            IN"-U --update              Updates libglyr if new version is available.\n"
            IN"-g --as-one              (Textitems only) Output all results as one (useful for tracklist and albumlist).\n"
            IN"-G --no-as-one           Disables --as-one (Default)\n"
            IN"-y --color               Enables colored output for glyrc (Default)\n"
            IN"-Y --no-color            Prints no colored output\n"
            IN"-s --musictree-path <p>  <p> is a path to your music directory. Glyr might fetch things like folger.jpg from there;\n"
            IN"-j --callback            Command: Set a bash command to be executed when a item is finished downloading;\n"
            IN"                                  All escapes mentioned in --write are supported too, and additonally:\n"
            IN"                                    * path : The path were the item was written to.\n"
            "\nDATABASE OPTIONS\n"
            IN"-c --cache <folder>      Creates or opens an existing cache at <folder>/metadata.db and lookups data from there.\n"
            IN"cache select [Query]     Selects data from the cache; you can use any other option behind this.\n"
            IN"cache delete [Query]     Instead of searching for this element, the element is deleted from the database. Needs --cache.\n"
            IN"cache list               List all items in the database (including the artist / album / title / type) - Needs --cache.\n"
            IN"\nExamples:\n\n"
            IN"    glyrc cover --artist SomeBand --album SomeAlbum\n"
            IN"   \n"
            IN"  DB:\n\n"
            IN"    glyrc lyrics -a A -b B --cache /tmp # Write found items to cache; create it if not existant\n"
            IN"    glyrc cache list --cache /tmp # List all in the cache\n"
            IN"    glyrc cache delete cover -a Equilibrium -b Sagas --cache /tmp # Delete artist/album\n"
            IN"    glyrc cache select lyrics -a Knorkator -t 'A' -n 2 --cache /tmp # Search for two items in cache and print them\n"
            IN"\n"
            IN"The return code will be 0 on success, 1 on any failure. No distinction is made which error happened.\n"
            IN"\n\n"
            "With each item received you get a link to the original source, please refer to the individual terms of use,\n"
            "copying and distributing of this data might be not allowed.\n"
            "A more detailed version of this help can be found online: https://github.com/sahib/glyr/wiki/Commandline-arguments\n"
           );

    cprint (DEFAULT,-1,s,"\nAUTHOR: (C) Christopher Pahl - 2011, <sahib@online.de>\n%s\n\n",glyr_version() );
    exit (EXIT_FAILURE);
#undef IN
}

////////////////////////////////////////////////////////////////

static void visualize_from_options (void)
{
    cprint (WHITE,-1,NULL,"# First line is the name of the fetcher you can use,\n"
            "# Second is the providername with the shortkey in []\n"
            "# Some unimportant information follows intented by '-'\n\n");

    GlyrFetcherInfo * info = glyr_info_get();
    if (info != NULL)
    {
        for (GlyrFetcherInfo * elem0 = info; elem0; elem0 = elem0->next)
        {
            cvprint (BLUE," %s => %d\n",elem0->name,elem0->type);
            for (GlyrSourceInfo * elem1 = elem0->head; elem1; elem1 = elem1->next)
            {
                cvprint (CYAN,"   # %s [%c]\n",elem1->name,elem1->key);
                cvprint (YELLOW,"     - Quality: %d\n",elem1->quality);
                cvprint (YELLOW,"     - Speed:   %d\n",elem1->speed);
                cvprint (YELLOW,"     - Type:    %s\n",glyr_get_type_to_string (elem1->type) );
            }

            cvprint (GREEN," + Requires: (%s%s%s)\n",
                     elem0->reqs & GLYR_REQUIRES_ARTIST ? "Artist " : "",
                     elem0->reqs & GLYR_REQUIRES_ALBUM  ? "Album "  : "",
                     elem0->reqs & GLYR_REQUIRES_TITLE  ? "Title"   : ""
                    );
            cvprint (MAGENTA," + Optional: (%s%s%s)\n",
                     elem0->reqs & GLYR_OPTIONAL_ARTIST ? "Artist " : "",
                     elem0->reqs & GLYR_OPTIONAL_ALBUM  ? "Album "  : "",
                     elem0->reqs & GLYR_OPTIONAL_TITLE  ? "Title"   : ""
                    );

            cvprint (BLUE,"\n///////////////////////////////\n\n");
        }

        cvprint (WHITE,"\nFollowing providers work with all types:\n");
        cvprint (WHITE,"  # local     [l] (A local SQLite DB cache; see --cache)\n");
        cvprint (WHITE,"  # musictree [t] (Gets items from your music directory (folder.jpg etc) --musictree-path)\n");
        cvprint (WHITE,"\nThe string 'all' in --from enables all providers.\n");
        cvprint (WHITE,"You can disable certain providers from this by prepending a '-':\n");
        cvprint (WHITE,"  \"all;-lastfm\"\n");
        cvprint (DEFAULT,"\n///////////////////////////////\n");
    }
    glyr_info_free (info);
}

////////////////////////////////////////////////////////////////

static void parse_commandline_general (int argc, char * const * argv, GlyrQuery * glyrs, callback_data_t * CBData, GlyrDatabase ** db)
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
        {"cache",         optional_argument, 0, 'c'},
        {"help",          no_argument,       0, 'h'},
        {"version",       no_argument,       0, 'V'},
        {"download",      no_argument,       0, 'd'},
        {"no-download",   no_argument,       0, 'D'},
        {"list",          no_argument,       0, 'L'},
        {"force-utf8",    no_argument,       0, '8'},
        {"as-one",        no_argument,       0, 'g'},
        {"no-as-one",     no_argument,       0, 'G'},
        {"only-lang",     no_argument,       0, 'o'},
        {"color",         no_argument,       0, 'y'},
        {"no-color",      no_argument,       0, 'Y'},
        {"artist",        required_argument, 0, 'a'},
        {"album",         required_argument, 0, 'b'},
        {"title",         required_argument, 0, 't'},
        {"minsize",       required_argument, 0, 'i'},
        {"maxsize",       required_argument, 0, 'e'},
        {"number",        required_argument, 0, 'n'},
        {"lang",          required_argument, 0, 'l'},
        {"fuzzyness",     required_argument, 0, 'z'},
        {"callback",      required_argument, 0, 'j'},
        {"musictree-path",required_argument, 0, 's'},
        {"normalization", required_argument, 0, 'N'},
        {0,               0,                 0, '0'}
    };

    opterr = 0;
    while (TRUE)
    {
        gint c;
        gint option_index = 0;
        if ( (c = getopt_long (argc, argv, "N:f:W:w:p:r:m:x:u:v:q:c::F:hVodDLa:b:t:i:e:s:n:l:z:j:k:8gGyY",long_options, &option_index) ) == -1)
        {
            break;
        }

        switch (c)
        {
        case 'w':
        {
            char * dirname = g_path_get_dirname (optarg);
            gsize opt_len = strlen (optarg);
            if (g_ascii_strncasecmp (optarg,"stdout",opt_len) == 0 ||
                g_ascii_strncasecmp (optarg,"stderr",opt_len) == 0 ||
                g_ascii_strncasecmp (optarg,"null",  opt_len) == 0 ||
                g_file_test (dirname,G_FILE_TEST_IS_DIR | G_FILE_TEST_EXISTS) == TRUE)
            {
                CBData->output_path = optarg;
                g_free (dirname);
            }
            else
            {
                g_free (dirname);
                cprint (DEFAULT,-1,NULL,"'%s' does not seem to be an valid directory!\n",optarg);
                exit (-1);
            }
            break;
        }
        case 'f':
            glyr_opt_from (glyrs,optarg);
            break;
        case 'v':
            glyr_opt_verbosity (glyrs,atoi (optarg) );
            break;
        case 'p':
            glyr_opt_parallel (glyrs,atoi (optarg) );
            break;
        case 'r':
            glyr_opt_redirects (glyrs,atoi (optarg) );
            break;
        case 'm':
            glyr_opt_timeout (glyrs,atoi (optarg) );
            break;
        case 'u':
            glyr_opt_useragent (glyrs,optarg);
            break;
        case 'x':
            glyr_opt_plugmax (glyrs,atoi (optarg) );
            break;
        case 'V':
            print_version (glyrs);
            break;
        case 'h':
            help_short (glyrs);
            break;
        case 'a':
            glyr_opt_artist (glyrs,optarg);
            break;
        case 'b':
            glyr_opt_album (glyrs,optarg);
            break;
        case 't':
            glyr_opt_title (glyrs,optarg);
            break;
        case 'i':
            glyr_opt_img_minsize (glyrs,atoi (optarg) );
            break;
        case 'e':
            glyr_opt_img_maxsize (glyrs,atoi (optarg) );
            break;
        case 'n':
            glyr_opt_number (glyrs,atoi (optarg) );
            break;
        case 'o':
            glyr_opt_lang_aware_only (glyrs,true);
            break;
        case 'd':
            glyr_opt_download (glyrs,true);
            break;
        case 'D':
            glyr_opt_download (glyrs,false);
            break;
        case 'c':
        {
            char * path = g_strdup(optarg);
            if(path == NULL) {
                path = create_cache_directory();
            }

            GlyrDatabase * new_db = glyr_db_init (path);
            if (db == NULL)
            {
                cprint (DEFAULT,-1,NULL,"Unable to open or create a database at specified path.\n");
                exit (EXIT_FAILURE);
            }
            *db = new_db;
            glyr_opt_db_autowrite (glyrs,TRUE);
            glyr_opt_lookup_db (glyrs,new_db);
            g_free(path);
        }
        break;
        case 'l':
            glyr_opt_lang (glyrs,optarg);
            break;
        case 'L':
            visualize_from_options();
            exit (0);
            break;
        case 'z':
            glyr_opt_fuzzyness (glyrs,atoi (optarg) );
            break;
        case 'j':
            CBData->exec_on_call = optarg;
            break;
        case 'k':
            glyr_opt_proxy (glyrs,optarg);
            break;
        case 'q':
            glyr_opt_qsratio (glyrs,atof (optarg) );
            break;
        case 'F':
            glyr_opt_allowed_formats (glyrs,optarg);
            break;
        case '8':
            glyr_opt_force_utf8 (glyrs,true);
            break;
        case 's':
            glyr_opt_musictree_path (glyrs,optarg);
            break;
        case 'g':
            CBData->as_one = true;
            break;
        case 'G':
            CBData->as_one = false;
            break;
        case 'y':
            enable_color (true);
            break;
        case 'Y':
            enable_color (false);
            break;
        case 'N':
            {
                GLYR_NORMALIZATION norm = 0;
                if (g_ascii_strcasecmp (optarg, "aggressive") == 0)
                    norm = GLYR_NORMALIZE_AGGRESSIVE;
                else if (g_ascii_strcasecmp (optarg, "moderate") == 0)
                    norm = GLYR_NORMALIZE_MODERATE;
                else if (g_ascii_strcasecmp (optarg, "none") == 0)
                    norm = GLYR_NORMALIZE_NONE;
                else 
                {
                    cprint (RED,-1,NULL,"No valid normalization level: '%s'\n", optarg);
                    exit(-1);
                }

                norm |= GLYR_NORMALIZE_ALL;

                glyr_opt_normalize(glyrs, norm);
            }
            break;
        case '?':
            cprint (RED,-1,NULL,"Option \"%s\" is not known\n",argv[optind-1]);
            exit (-1);
            break;
        default:
            cprint (RED,-1,NULL,"Unknown Error while parsing commandline arguments.\n");
            exit (-1);
        } // end switch()
    } // end while()
}

////////////////////////////////////////////////////////////////
// --------------------------------------------------------- //
////////////////////////////////////////////////////////////////

static GLYR_ERROR callback (GlyrMemCache * c, GlyrQuery * s)
{
    callback_data_t * CBQueryData = s->callback.user_pointer;
    g_assert (CBQueryData);

    char * write_to_path = NULL;

    increment_item_counter();

    /* write out 'live' */
    if (CBQueryData->output_path != NULL)
    {
        gsize write_len = strlen (CBQueryData->output_path);
        if (g_ascii_strncasecmp (CBQueryData->output_path,"stdout",write_len) == 0 ||
            g_ascii_strncasecmp (CBQueryData->output_path,"stderr",write_len) == 0 ||
            g_ascii_strncasecmp (CBQueryData->output_path,"null",  write_len) == 0)
        {
            glyr_cache_write (c,CBQueryData->output_path);
        }
        else
        {
            if (g_file_test (CBQueryData->output_path,G_FILE_TEST_IS_DIR) == TRUE)
            {
                char * filename  = escape_colon_expr (":artist:_:album:_:title:_:type:_:number:.:format:",s,c);
                char * directory = escape_colon_expr (CBQueryData->output_path,s,c);
                write_to_path = g_strdup_printf ("%s%s%s",directory,
                                                 g_str_has_suffix (directory,G_DIR_SEPARATOR_S) ? "" : G_DIR_SEPARATOR_S,
                                                 filename);

                g_free (filename);
                g_free (directory);
            }
            else
            {
                write_to_path = escape_colon_expr (CBQueryData->output_path,s,c);
            }

            if (glyr_cache_write (c,write_to_path) == -1)
            {
                cprint (RED,1,s,"Error: Writing data to <%s> failed.\n",write_to_path);
            }

            set_write_path (write_to_path);

            /* call a program if any specified */
            if (CBQueryData->exec_on_call != NULL)
            {
                char * command = escape_colon_expr (CBQueryData->exec_on_call,s,c);

                /* Call that command */
                int exitVal = system (command);
                if (exitVal != EXIT_SUCCESS)
                {
                    cprint (YELLOW,3,s,"Warning: cmd returned a value != EXIT_SUCCESS\n");
                }
                g_free (command);
            }
        }
    }

    /* Text Represantation of this item */
    cprint (BLUE,1,s,"\n///// ITEM #%d /////\n",get_item_counter() );
    if (write_to_path != NULL)
    {
        cprint (DEFAULT,1,s,"WRITE to '%s'\n",write_to_path);
        //g_free(write_to_path);
    }

    if (s->verbosity > 0)
    {
        glyr_cache_print (c);
    }
    cprint (BLUE,1,s,"\n////////////////////\n");

    return GLYRE_OK;
}

////////////////////////////////////////////////////////////////
// --------------------------------------------------------- //
////////////////////////////////////////////////////////////////

GLYR_GET_TYPE get_type_from_string (const char * string)
{
    GLYR_GET_TYPE result = GLYR_GET_UNKNOWN;
    if (g_ascii_strncasecmp (string,"none",4) == 0)
        return GLYR_GET_ANY;

    GlyrFetcherInfo * info = glyr_info_get();
    if (info != NULL)
    {
        for (GlyrFetcherInfo * elem = info; elem; elem = elem->next)
        {
            if (g_ascii_strncasecmp (elem->name,string,strlen (elem->name) ) == 0)
            {
                result = elem->type;
                break;
            }
        }
    }
    else
    {
        cprint (DEFAULT,-1,NULL,"Warning: Can't get type information. Probably a bug.\n");
    }
    glyr_info_free (info);
    return result;
}

////////////////////////////////////////////////////////////////
// --------------------------------------------------------- //
////////////////////////////////////////////////////////////////

static gint db_foreach_callback (GlyrQuery * q, GlyrMemCache * c, void * p)
{
    puts ("\n////////////////////////////////////\n");
    cprint (DEFAULT,-1,NULL,"%s :: %s :: %s :: %s\n",q->artist,q->album,q->title,glyr_get_type_to_string (q->type) );
    glyr_cache_print (c);

    int * i = p;
    *i += 1;
    return 0;
}

//////////////////////////////////////

static void do_iterate_over_db (GlyrDatabase * db)
{
    gint db_item_counter = 0;
    glyr_db_foreach (db,db_foreach_callback,&db_item_counter);
    cprint (DEFAULT,-1,NULL,"\n-----------------------------------\n");
    cprint (DEFAULT,-1,NULL,"=> In total %d items in database.",db_item_counter);
    cprint (DEFAULT,-1,NULL,"\n-----------------------------------\n");
    cprint (DEFAULT,-1,NULL,"\n");
}

//////////////////////////////////////

static int do_cache_interface (const char * operation, GlyrQuery * q, GlyrDatabase * db, callback_data_t * cbdata)
{
    const char list_op[] = "list";
    const char slct_op[] = "select";
    const char dele_op[] = "delete";

    g_assert (q);

    if (db == NULL)
    {
        cvprint (RED,"No database specified. Use --cache.\n");
        return EXIT_FAILURE;
    }

    ////////////////////////////////

    if (g_ascii_strncasecmp (operation,list_op,sizeof (list_op) - 1) == 0)
    {
        do_iterate_over_db (db);
        return EXIT_SUCCESS;
    }

    ////////////////////////////////

    if (g_ascii_strncasecmp (operation,dele_op,sizeof (dele_op) - 1) == 0)
    {
        int killed_items = glyr_db_delete (db,q);
        cvprint (BLUE,"Deleted %d items from the db.",killed_items);
        return killed_items > 0;
    }

    ////////////////////////////////

    if (g_ascii_strncasecmp (operation,slct_op,sizeof (slct_op) - 1) == 0)
    {
        GlyrMemCache * list = glyr_db_lookup (db,q);
        GlyrMemCache * iter = list;

        if(iter == NULL)  {
            return EXIT_FAILURE;
        }

        q->callback.user_pointer = cbdata;
        while (iter != NULL) {
            callback(iter, q);
            iter = iter->next;
        }

        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}


//////////////////////////////////////

static bool parse_type_argument (const char * argvi, GlyrQuery * qp)
{
    bool retv = false;

    g_assert (qp);

    if (argvi != NULL)
    {
        GLYR_GET_TYPE type = get_type_from_string (argvi);
        if (type == GLYR_GET_UNKNOWN)
        {
            cvprint (RED,"Error: \"%s\" is not a know getter.\n",argvi);
            cvprint (DEFAULT,"Currently available are:\n");
            GlyrFetcherInfo * info = glyr_info_get();
            GlyrFetcherInfo * iter = info;
            while (iter != NULL)
            {
                cvprint (BLUE,"  * %s\n",iter->name);
                iter = iter->next;
            }
            glyr_info_free (info);

            suggest_other_getter (qp,argvi);
            cvprint (DEFAULT,"\nSee `glyrc -L` for a detailed list\n");
            retv = false;

        }
        else
        {
            glyr_opt_type (qp,type);
            retv = true;
        }
    }

    return retv;
}

//////////////////////////////////////

static GlyrMemCache * concatenate_list (GlyrMemCache * list)
{
    if (list == NULL)
        return NULL;

    GlyrMemCache * iter = list;
    GlyrMemCache * retv = glyr_cache_copy (list);
    gchar * concat_data = g_strdup ("");

    retv->duration = 0;

    while (iter != NULL)
    {
        gchar * tmp = g_strdup_printf ("%s%s\n",concat_data,iter->data);
        g_free (concat_data);
        concat_data = tmp;
        retv->duration += iter->duration;
        iter = iter->next;
    }

    retv->next = NULL;
    retv->prev = NULL;

    glyr_cache_set_data (retv,concat_data,-1);
    glyr_free_list (list);
    return retv;
}

//////////////////////////////////////

static void global_cleanup (void)
{
    set_write_path (NULL);
}

//////////////////////////////////////

int main (int argc, char * argv[])
{
    /* Set the loghandler (used by cprint(DEFAULT,)) */
    g_log_set_handler (LOG_DOMAIN ,
                       G_LOG_LEVEL_MASK |
                       G_LOG_FLAG_FATAL |
                       G_LOG_FLAG_RECURSION,
                       log_func, NULL);

    atexit (global_cleanup);
    init_signals();

    /* Assume failure, as always in life */
    gint exit_code = EXIT_FAILURE;

    /* Init / Destroy of libglyr.
     * You _have_ to call this before making any calls
     * Both functions are not threadsafe!
     * */
    glyr_init();
    atexit (glyr_cleanup);

    if (argc >= 2 && argv[1][0] != '-' && strcmp (argv[1],"cache") != 0)
    {
        callback_data_t CBQueryData = INIT_CALLBACK_DATA;
        GlyrDatabase * db = NULL;
        GlyrQuery my_query;

        glyr_query_init (&my_query);
        glyr_opt_verbosity (&my_query,2);

        init_cancel_handler (&my_query);

        if (parse_type_argument (argv[1],&my_query) )
        {
            int length = -1;
            GLYR_ERROR get_error = GLYRE_OK;

            parse_commandline_general (argc-1, argv+1, &my_query,&CBQueryData,&db);
            SET_IF_NULL (CBQueryData.output_path,".");

            if (CBQueryData.as_one == false || glyr_type_is_image (my_query.type) )
            {
                glyr_opt_dlcallback (&my_query, callback, &CBQueryData);
            }

            /* Doing the 'hard' work here. */
            GlyrMemCache * my_list = glyr_get (&my_query, &get_error, &length);

            if (my_list != NULL)
            {
                exit_code = EXIT_SUCCESS;

                if (CBQueryData.as_one && glyr_type_is_image (my_query.type) == false)
                {
                    GlyrMemCache * concat = concatenate_list (my_list);
                    glyr_opt_dlcallback (&my_query, callback, &CBQueryData);
                    callback (concat,&my_query);

                    glyr_cache_free (concat);
                }
                else
                {
                    /* Free all downloaded buffers */
                    glyr_free_list (my_list);
                    cprint (YELLOW,2,&my_query,"# => %d item%s in total.\n",length,length == 1 ? "" : "s");
                }
            }
            else if (get_error == GLYRE_NO_PROVIDER)
            {
                cvprint (RED,"Error `--from %s` does not contain any valid provider.\n",my_query.from);
                cvprint (RED,"      See `glyrc -L` for a list.\n");
                suggest_other_provider (&my_query,my_query.from);
            }
            else if (get_error != GLYRE_OK)
            {
                cprint (RED,1,&my_query,"Error: %s\n",glyr_strerror (get_error) );
            }
        }

        glyr_db_destroy (db);
        glyr_query_destroy ( &my_query);
    }
    else if (argc >= 2 && strcmp (argv[1],"cache") == 0)
    {
        if (argc >= 3)
        {
            GlyrQuery db_query;
            GlyrDatabase * db = NULL;
            callback_data_t CBQueryData = INIT_CALLBACK_DATA;
            SET_IF_NULL (CBQueryData.output_path,".");

            glyr_query_init (&db_query);
            glyr_opt_verbosity (&db_query,2);

            if (argc >= 4 && strcmp("list", argv[2]) != 0)
            {
                parse_type_argument (argv[3],&db_query);
            }

            parse_commandline_general (argc-2, argv+2,&db_query,&CBQueryData,&db);
            do_cache_interface (argv[2],&db_query,db, &CBQueryData);
        }
        else
        {
            cvprint (RED,"Error: 'cache': needs another argument:\n");
            cvprint (RED,"  * select [opts]\n");
            cvprint (RED,"  * delete [opts]\n");
            cvprint (RED,"  * list\n");
        }
    }
    else if (argc >= 2 && (!strcmp (argv[1],"-U") || !strcmp (argv[1],"--update") ) )
    {
        update();
    }
    else if (argc >= 2 && (!strcmp (argv[1],"-V") || !strcmp (argv[1],"--version") ) )
    {
        print_version (NULL);
    }
    else if (argc >= 2 && (!strcmp (argv[1],"-L") || !strcmp (argv[1],"--list") ) )
    {
        visualize_from_options();
    }
    else
    {
        help_short (NULL);
    }

    return exit_code;
}

////////////////////////////////////////////////////////////////
// ------------------End of program-------------------------- //
////////////////////////////////////////////////////////////////
