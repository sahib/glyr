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
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <libgen.h>
#include <signal.h>

#include "../lib/glyr.h"

// also includes glyr's stringlib
// you shouldnt do this yourself, but
// implementing functions twice is silly
// (in this case at least )
#include "../lib/stringlib.h"
// For glyr_message. Also don't use it.
#include "../lib/core.h"

#include <dirent.h>
#include <sys/types.h>

bool update = false;
const char * default_path = ".";

//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static void print_version(GlyQuery * s)
{
    glyr_message(-1,s,stdout, C_G"%s\n\n"C_,Gly_version());
    glyr_message(-1,s,stderr, C_"This is still beta software, expect quite a lot bugs.\n");
    glyr_message(-1,s,stderr, C_"Email bugs to <sahib@online.de> or use the bugtracker\n"
                 C_"at https://github.com/sahib/glyr/issues - Thank you! \n");

    exit(0);
}

//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

#define _S "\t"
static void list_provider_at_id(int id, int min_align,GlyQuery * s)
{
    GlyPlugin * cp = Gly_get_provider_by_id(id);

    if(cp != NULL)
    {
        int i = 0;
        for(i = 0; cp[i].name != NULL; i++)
        {
            int align = min_align - strlen(cp[i].name);
            int a = 0;
            glyr_message(-1,s,stdout,C_G _S"%s"C_,cp[i].name);
            for(a = 0; a < align; a++)
                glyr_message(-1,s,stdout," ");

            glyr_message(-1,s,stdout,C_" ["C_R"%s"C_"]%s",cp[i].key, (id == GET_UNSURE) ? "" : " in groups "C_Y"all"C_);
            int b = 1, j = 0;
            for(b = 1, j = 0; id != GET_UNSURE &&  b <= GRP_ALL; j++)
            {
                if(b & cp[i].gid)
                {
                    glyr_message(-1,s,stdout,C_","C_Y"%s",Gly_groupname_by_id(b));
                }
                b <<= 1;
            }
            glyr_message(-1,s,stdout,"\n");
        }
        free(cp);
    }
}
/*
static void usage(GlyQuery * s)
{
    glyr_message(-1,s,stdout,C_B"USAGE:\n"C_ _S"glyrc "C_Y"GETTER"C_" [OPTIONS...]\n\n");
    glyr_message(-1,s,stdout,"glyrc downloads variouse sorts of musicrelated metadata.\n");
    glyr_message(-1,s,stdout,C_Y"GETTER"C_" is the type of metadata to download, it must be one of: \n");
    list_provider_at_id(GET_UNSURE, 10,s);

#define GET_C C_G
#define OPT_C C_C
#define OPT_A OPT_C"\n    "
    glyr_message(-1,s,stdout,"\nThe getter has to be always the very first argument given, and may require other arguments\n");
    glyr_message(-1,s,stdout,"A short note to the terminology: a provider is a source glyrc may download data from.\n");
    glyr_message(-1,s,stdout,"A group is a collection of providers, each getter has at least the groups all,safe and unsafe,\n");
    glyr_message(-1,s,stdout,"Please note that providers and groups may differ heavily from getter to getter!\n");
    glyr_message(-1,s,stdout,"you can modify glyrc's providerlist by passing the names (or their shortcuts) to --from.\n");
    glyr_message(-1,s,stdout,"To find out what getter needs what argument see below: ("OPT_C"-n"C_" is the max number of items)\n");
    glyr_message(-1,s,stdout,C_B"\nCOVER\n"C_);
    glyr_message(-1,s,stdout,_S"Download "OPT_C"-n"C_" albumart images of the artist and album specified with "OPT_C"-a and -b\n"C_);
    glyr_message(-1,s,stdout,_S"The filetype of the image is unspecified (as libglyr often could only vaguely guess)\n");
    glyr_message(-1,s,stdout,_S"and may be determinded by its header; glyrc saves it as '.img in all cases.'\n");
    glyr_message(-1,s,stdout,_S"Currently "OPT_C"--from"C_" takes the following strings here:\n\n");
    list_provider_at_id(GET_COVER,12,s);
    glyr_message(-1,s,stdout,C_B"LYRICS\n"C_);
    glyr_message(-1,s,stdout,_S"Download "OPT_C"-n"C_" lyrics of the artist and title specified with "OPT_C"-a"C_" and "OPT_C"-t\n"C_);
    glyr_message(-1,s,stdout,_S"The album ("OPT_C"-b"C_") is optional and may be used by some plugins.\n");
    glyr_message(-1,s,stdout,_S"Currently "OPT_C"--from"C_" takes the following strings here:\n\n");
    list_provider_at_id(GET_LYRIC,13,s);
    glyr_message(-1,s,stdout,C_B"PHOTOS\n"C_);
    glyr_message(-1,s,stdout,_S"Download "OPT_C"-n"C_" photos that are related to the artist given by "OPT_C"-a"C_".\n");
    glyr_message(-1,s,stdout,_S"Currently "OPT_C"--from"C_" takes the following strings here:\n\n");
    list_provider_at_id(GET_PHOTO,12,s);
    glyr_message(-1,s,stdout,C_B"AINFO\n"C_);
    glyr_message(-1,s,stdout,_S"Download "OPT_C"-n"C_" artist descriptions of the artist given by "OPT_C"-a"C_".\n");
    glyr_message(-1,s,stdout,_S"Apart from the name, a similiarity rating from 0.0 to 1.0, a URL to last.fm page\n");
    glyr_message(-1,s,stdout,_S"and a bunch of URLs to images of the similiar artist in ascending size.\n");
    glyr_message(-1,s,stdout,_S"Currently "OPT_C"--from"C_" takes the following strings here:\n\n");
    list_provider_at_id(GET_AINFO,12,s);
    glyr_message(-1,s,stdout,C_B"\nSIMILIAR\n"C_);
    glyr_message(-1,s,stdout,_S"Download "OPT_C"-n"C_" information about similiar artist to the one given with -a "OPT_C"-a"C_".\n");
    glyr_message(-1,s,stdout,_S"Currently "OPT_C"--from"C_" takes the following strings here:\n\n");
    list_provider_at_id(GET_SIMILIAR,12,s);
    glyr_message(-1,s,stdout,C_B"\nTRACKLIST\n"C_);
    glyr_message(-1,s,stdout,_S"Get a list of tracks by artist given by -a and album by -b\n");
    list_provider_at_id(GET_TRACKLIST,12,s);
    glyr_message(-1,s,stdout,C_B"\nALBUMLIST\n"C_);
    glyr_message(-1,s,stdout,_S"Get a list of albums by artist given by -a\n");
    list_provider_at_id(GET_ALBUMLIST,12,s);
    glyr_message(-1,s,stdout,OPT_A"-f --from <prov>\n"C_);
    glyr_message(-1,s,stdout,_S"Set the sources (providers) you want to query.\n");
    glyr_message(-1,s,stdout,_S"The string you have to provide cotains the names of the providers or a group\n");
    glyr_message(-1,s,stdout,_S"(or their shortcuts), seperated by a \"%s\" and prepended by a + or -,\n", DEFAULT_FROM_ARGUMENT_DELIM);
    glyr_message(-1,s,stdout,_S"which adds or deletes this source to/from the current state.\n");
    glyr_message(-1,s,stdout,_S"An example would be: \"+all%s-special%s+d\"\n",DEFAULT_FROM_ARGUMENT_DELIM,DEFAULT_FROM_ARGUMENT_DELIM );
    glyr_message(-1,s,stdout,_S _S "+all     : adds everything.\n");
    glyr_message(-1,s,stdout,_S _S "-special : subtract the members of group 'special'.\n");
    glyr_message(-1,s,stdout,_S _S "+d       : add the provider 'd' (discogs, see above)\n");
    glyr_message(-1,s,stdout,OPT_A"-n --number <int>\n"C_);
    glyr_message(-1,s,stdout,_S"Maximum number of items a getter may download.\n");
    glyr_message(-1,s,stdout,_S"Default is %d.\n", DEFAULT_NUMBER);
    glyr_message(-1,s,stdout,OPT_A"-x --plugmax <int>\n"C_);
    glyr_message(-1,s,stdout,_S"Maximum number of items a plugin may download.\n");
    glyr_message(-1,s,stdout,_S"Default is %d.\n", DEFAULT_PLUGMAX);
    glyr_message(-1,s,stdout,OPT_A"-u --update\n"C_);
    glyr_message(-1,s,stdout,_S"Update file even if already present.\n");
    glyr_message(-1,s,stdout,_S"Default is %s.\n",update ? "true" : "false");
    glyr_message(-1,s,stdout,OPT_A"-d --nodownload\n"C_);
    glyr_message(-1,s,stdout,_S"Do not download final result, only print URL.\n");
    glyr_message(-1,s,stdout,_S"This only works for images, as lyrics and ainfo do not neccesarely have a concrete URL.\n");
    glyr_message(-1,s,stdout,_S"Use this to turn glyrc into some sort of music-metadate search engine.\n");
    glyr_message(-1,s,stdout,_S"Default is %s.\n",DEFAULT_DOWNLOAD ? "true" : "false");
    glyr_message(-1,s,stdout,OPT_A"-w --write <dir>\n"C_);
    glyr_message(-1,s,stdout,_S"Write all files to the directory <dir>\n");
    glyr_message(-1,s,stdout,_S"The filenames itself is determined by the artist,album, title depending on <GET>, see the also the FILES section.\n");
    glyr_message(-1,s,stdout,_S"The special value \"stdout\" will print the data directly to stdout, \"stderr\" to stderr\n"_S"and \"null\" will print nothing.\n");
    glyr_message(-1,s,stdout,_S"Default is '%s'\n",default_path);
    glyr_message(-1,s,stdout,C_B"\nLIBCURL OPTIONS\n"C_);
    glyr_message(-1,s,stdout,OPT_A"-p --parallel <int>\n"C_);
    glyr_message(-1,s,stdout,_S"Download max. <int> files in parallel if there's more than one to download.\n");
    glyr_message(-1,s,stdout,_S"Set to 1 for serial download; Default is %d.\n", DEFAULT_PARALLEL);
    glyr_message(-1,s,stdout,OPT_A"-r --redirects <int>\n"C_);
    glyr_message(-1,s,stdout,_S"Allow max. <int> redirects. .\n");
    glyr_message(-1,s,stdout,_S"Default is %d.\n", DEFAULT_REDIRECTS);
    glyr_message(-1,s,stdout,OPT_A"-m --timeout <int>\n"C_);
    glyr_message(-1,s,stdout,_S"Maximum number of <int> seconds to wait before cancelling a download.\n");
    glyr_message(-1,s,stdout,_S"Default is %d.\n", DEFAULT_TIMEOUT);
    glyr_message(-1,s,stdout,C_B"\nMISC OPTIONS\n"C_);
    glyr_message(-1,s,stdout,OPT_A"-V --version\n"C_);
    glyr_message(-1,s,stdout,_S"Print version string and exit.\n");
    glyr_message(-1,s,stdout,OPT_A"-h --help\n"C_);
    glyr_message(-1,s,stdout,_S"Print this help and exit.\n");
    glyr_message(-1,s,stdout,OPT_A"-c --color\n"C_);
    glyr_message(-1,s,stdout,_S"Enables colored console output (Unix only).\n");
    glyr_message(-1,s,stdout,_S"Default is %s.\n", PRT_COLOR ? "true" : "false");
    glyr_message(-1,s,stdout,OPT_A"-v --verbosity <int>\n"C_);
    glyr_message(-1,s,stdout,_S"Level of verbosity:\n");
    glyr_message(-1,s,stdout,_S _S"-v0: Print nothing but fatal errors.\n");
    glyr_message(-1,s,stdout,_S _S"-v1: Print only basic information.\n");
    glyr_message(-1,s,stdout,_S _S"-v2: Print informative output. (default)\n");
    glyr_message(-1,s,stdout,_S _S"-v3: Enable debugging messages.\n");
    glyr_message(-1,s,stdout,_S _S"-v4: Enable libcurl-debugging messages.\n");
    glyr_message(-1,s,stdout,C_B"\nPLUGIN OPTIONS\n"C_);
    glyr_message(-1,s,stdout,OPT_A"-a --artist <string>; -b --album <string>; -t --title <string>\n"C_);
    glyr_message(-1,s,stdout,_S"Depending on the getter you have to provide information on what to search.\n");
    glyr_message(-1,s,stdout,_S"Please refer to the getter description to find what argument is needed for what getter.\n");
    glyr_message(-1,s,stdout,OPT_A"-i --minsize\n"C_);
    glyr_message(-1,s,stdout,_S"For cover only: The minimum size a coverimage may have\n");
    glyr_message(-1,s,stdout,_S"A value of -1 will disable the sizecheck.\n");
    glyr_message(-1,s,stdout,_S"Note that this value is only a 'suggestion' for libglyr,\n");
    glyr_message(-1,s,stdout,_S"but it should work in 95%% of all cases.\n");
    glyr_message(-1,s,stdout,_S"Default is %d.\n",DEFAULT_CMINSIZE);
    glyr_message(-1,s,stdout,OPT_A"-e --maxsize\n"C_);
    glyr_message(-1,s,stdout,_S"For cover only: The maximum size a coverimage may have (see above)\n");
    glyr_message(-1,s,stdout,_S"A value of -1 will disable the sizecheck.\n");
    glyr_message(-1,s,stdout,_S"Default is %d.\n",DEFAULT_CMAXSIZE);
    glyr_message(-1,s,stdout,OPT_A"-l --lang <l>\n"C_);
    glyr_message(-1,s,stdout,_S"For cover:\n"_S _S"decides which amazon / google server is queried,\n"_S _S"what may have impact on search results.\n\n");
    glyr_message(-1,s,stdout,_S"For ainfo's last.fm plugin:\n"_S _S"Decide in which language results are returned.\n\n");
    glyr_message(-1,s,stdout,_S"The language is given by ISO 639-1 codes;\n");
    glyr_message(-1,s,stdout,_S"Examples are: us,ca,uk,fr,de,jp\n");
    glyr_message(-1,s,stdout,C_B"\nEXAMPLES\n"C_);
    glyr_message(-1,s,stdout,_S C_Y"glyrc cover -a Equilibrium -b \"Turis Fratyr\"\n"C_);
    glyr_message(-1,s,stdout,_S _S"# Download the cover art of the album 'Turis Fratyr' by the band equilibrium,\n");
    glyr_message(-1,s,stdout,_S _S"# save it to Equilibrium_Turis+Fratyr_0.img\n");
    glyr_message(-1,s,stdout,_S C_Y"glyrc cover -a Equilibrium -b Rekreatur -n 5 -i 100 -e 250 --from \"amazon\"\n"C_);
    glyr_message(-1,s,stdout,_S _S"# Download 5x (different) images of 'Rekreatur',\n");
    glyr_message(-1,s,stdout,_S _S"# with the dimensions in between 100x100 and 250x250 from amazon\n");
    glyr_message(-1,s,stdout,_S C_Y"glyrc lyrics -a Equilibrium -t \"Blut im Auge\"\n"C_);
    glyr_message(-1,s,stdout,_S _S"# Download lyrics of the Song \"Blut im Auge\" by Equilibrium,\n");
    glyr_message(-1,s,stdout,_S _S"# show it in stdout, and write it to Equilibrium_Blut+im+Auge_0.lyrics\n");
    glyr_message(-1,s,stdout,_S C_Y"glyrc ainfo -a \"Justin Bieber\"\n"C_);
    glyr_message(-1,s,stdout,_S _S"# This produces nothing interesting (but hopefully a Segfault).\n");
    glyr_message(-1,s,stdout,_S _S"# If not it will print all the things you always wanted to know about Justin Bieber.\n");
    glyr_message(-1,s,stdout,_S _S"# (In german Language)\n");
    glyr_message(-1,s,stdout,_S C_Y"glyrc photos -a \"Die Apokalyptischen Reiter\" -v0 -n 5\n"C_);
    glyr_message(-1,s,stdout,_S _S"# Downloads 5 photos showing the band \"Die Apokalyptischen Reiter\"\n");
    glyr_message(-1,s,stdout,_S C_Y"glyrc review -a \"Equilibrium\" -b \"Sagas\" -w stdout -v0\n"C_);
    glyr_message(-1,s,stdout,_S _S"# Dumps the review of Equilibrium Sagas to stdout\n");
    glyr_message(-1,s,stdout,_S C_Y"glyrc similiar -a \"Equilibrium\" -w stdout -v0 -n 5\n"C_);
    glyr_message(-1,s,stdout,_S _S"# Show the 5 most similiar artist to Equilibrium (according to last.fm...)\n");
    glyr_message(-1,s,stdout,_S _S"# Every set includes the name, matchrate (from 0.0 to 1.0),\n");
    glyr_message(-1,s,stdout,_S _S"# a link to the last.fm page, and a link to a pressphoto (different sizes)\n");
    glyr_message(-1,s,stdout,C_B"\nFILES\n"C_);
    glyr_message(-1,s,stdout,_S"Everything is stored in a file with the pattern $save_dir/$artist_($album|$title)_suffix.type\n");
    glyr_message(-1,s,stdout,_S"Spaces and Slashes in artist/album/title are escaped with a '+'. ($num is the itemcounter)\n");
    glyr_message(-1,s,stdout,_S _S"$dir/$artist_$album_cover_$num.jpg\n");
    glyr_message(-1,s,stdout,_S _S"$dir/$artist_$title_lyrics_$num.txt\n");
    glyr_message(-1,s,stdout,_S _S"$dir/$artist_photos_$num.jpg\n");
    glyr_message(-1,s,stdout,_S _S"$dir/$artist_ainfo_$num.txt\n");
    glyr_message(-1,s,stdout,_S _S"$dir/$artist_similiar_$num.txt\n");
    glyr_message(-1,s,stdout,_S _S"$dir/$artist_$album_review_$num.txt\n");
    glyr_message(-1,s,stdout,C_B"\nAUTHOR\n"C_);
    glyr_message(-1,s,stdout,_S"(C) Christopher Pahl - 2011\n");
    glyr_message(-1,s,stdout,_S"See the AUTHORS file that comes in glyr's distribution for a complete list.\n");
    glyr_message(-1,s,stdout,_S"See also COPYING to know about your rights.\n");
    glyr_message(-1,s,stdout,_S"\n");

    print_version(s);
    exit(0);
}
#undef S
#undef OPT_C
#undef GET_C
*/
//* ------------------------------------------------------- */

static void sig_handler(int signal)
{
    switch(signal)
    {
    case SIGINT:
        glyr_message(-1,NULL,stderr,C_"Received keyboard interrupt!\n");
        break;
    case SIGSEGV :
        glyr_message(-1,NULL,stderr,C_R"\nFATAL: "C_"libglyr crashed due to a Segmentation fault.\n");
        glyr_message(-1,NULL,stderr,C_"       This is entirely the fault of the libglyr developers. Yes, we failed. Sorry. Now what to do:\n"); 
        glyr_message(-1,NULL,stderr,C_"       It would be just natural to blame us now, so just visit <https://github.com/sahib/glyr/issues>\n"); 
        glyr_message(-1,NULL,stderr,C_"       and throw hard words like 'backtrace', 'bug report' or even the 'command I issued' at them.\n");
        glyr_message(-1,NULL,stderr,C_"       The libglyr developers will try to fix it as soon as possible so you stop pulling their hair.\n");

        glyr_message(-1,NULL,stderr,C_"\n(Thanks, and Sorry for any bad feelings.)\n");
        break;
    }
    exit(-1);
}


//* ------------------------------------------------------- */
// -------------------------------------------------------- //
/* -------------------------------------------------------- */

static bool set_get_type(GlyQuery * s, const char * arg)
{
    bool result = true;

    if(!arg)
    {
        return result;
    }
    // get list of avaliable commands
    GlyPlugin * plist = Gly_get_provider_by_id(GET_UNSURE);
    if(plist)
    {
        int i = 0;
        for(; plist[i].name; i++)
        {
            if(!strcmp(arg,plist[i].name) || !strcmp(arg,plist[i].key))
            {
                GlyOpt_type(s,(long)plist[i].gid);
                free(plist);
                return result;
            }
        }

        result = false;

        glyr_message(2,s,stderr,"Sorry, I don't know of a getter called '%s'...\n\n",arg);
        glyr_message(2,s,stderr,"Avaliable getters are: \n");
        glyr_message(2,s,stderr,"---------------------- \n");

        list_provider_at_id(GET_UNSURE,7,s);

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

static void search_similiar_providers(const char * providers, GlyQuery * s)
{
    GlyPlugin * plist = Gly_get_provider_by_id(s->type);
    if(plist)
    {

        size_t length = strlen(providers);
        size_t offset = 0;
        char * name = NULL;

        while( (name = get_next_word(providers,DEFAULT_FROM_ARGUMENT_DELIM,&offset,length)))
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
                            glyr_message(2,s,stderr,C_R"* "C_"Did you mean");
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

        glyr_message(2,s,stderr,C_R"* "C_"Must be one of:\n");
        list_provider_at_id(s->type,15,s);
        glyr_message(2,s,stderr,"\n");

        free(plist);
    }
}

//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */


static void suggest_other_options(int m, int argc, char * const * argv, int at, struct option * long_options, GlyQuery * s)
{
    bool dym = false;

    char * argument = argv[at];
    while(*argument && *argument == '-')
        argument++;

    glyr_message(1,s,stderr,C_R"*"C_" Unknown option or missing argument: %s\n",argv[at]);
    if(argument && *argument)
    {
        int i = 0;
        for(i = 0; i < m && long_options[i].name; i++)
        {
            if(levenshtein_strcmp(argument,long_options[i].name) < 3 || *(argument) == long_options[i].val)
            {
                if(!dym)
                {
                    glyr_message(1,s,stderr,C_G"*"C_" Did you mean");
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

static bool check_if_dir(const char * path)
{
    if(strcasecmp(optarg,"stdout") && strcasecmp(optarg,"stderr") && strcasecmp(optarg,"null"))
    {
        DIR * dummy = opendir(path);
        if(dummy)
        {
            closedir(dummy);
            return true;
        }
        else
        {
            return false;
        }
    }
    return true;
}

/* --------------------------------------------------------- */

void help_short(GlyQuery * s)
{
    glyr_message(-1,s,stderr,"Usage: glyrc [GETTER] (options)\n\nwhere [GETTER] must be one of:\n");

    list_provider_at_id(GET_UNSURE,10,s);

    glyr_message(-1,s,stderr,"\nIf you're viewing this helptext the first time,\n"
                 "you probably want to view --usage, which has more details & examples\n");

#define IN "    "
    glyr_message(-1,s,stderr,"\n\nOPTIONS:\n"
                 IN"-f --from  <s>        Providers from where to get metadata. Refer to the list at the end of this text.\n"
                 IN"-w --write <d>        Write metadata to dir <d>, special values stdout, stderr and null are supported\n"
                 IN"-p --parallel <i>     Integer. Define the number of downloads that may be performed in parallel.\n"
                 IN"-r --redirects        Integer. Define the number of redirects that are allowed.\n"
                 IN"-m --timeout          Integer. Define the maximum number in seconds after which a download is cancelled.\n"
                 IN"-x --plugmax          Integer. Maximum number od download a plugin may deliever. Use to make results more vary.\n"
                 IN"-v --verbosity        Integer. Set verbosity from 0 to 4. See --usage for details.\n"
                 IN"-u --update           Also download metadata if files are already in path (given by -w or '.')\n"
                 IN"-U --skip-update      Do not download data if already present.\n"
                 IN"-h --help             This text you unlucky wanderer are viewing.\n"
                 IN"-H --usage            A more detailed version of this text.\n"
                 IN"-V --version          Print the version string.\n"
                 IN"-c --color            Enable colored output (Unix only)\n"
                 IN"-C --skip-color       Disable colored output. (Unix only)\n"
                 IN"-d --download         Download Images.\n"
                 IN"-D --skip-download    Don't download images, but return the URLs to them (act like a search engine)\n"
                 IN"-g --groups           Enable grouped download (Slower but more accurate, as quality > speed)\n"
                 IN"-G --skip-groups      Query all providers at once. (Faster but may deliever weird results)\n"
		 IN"-y --twincheck        Check for duplicate URLs, yes by default\n"
		 IN"-y --skip-twincheck   Disable URLduplicate check.\n"
                 IN"-a --artist           Artist name (Used by all plugins)\n"
                 IN"-b --album            Album name (Used by cover,review,lyrics)\n"
                 IN"-t --title            Songname (used mainly by lyrics)\n"
                 IN"-e --maxsize          (cover only) The maximum size a cover may have.\n"
                 IN"-i --minsize          (cover only) The minimum size a cover may have.\n"
                 IN"-n --number           Download max. <n> items. Amount of actual downloaded items may be less.\n"
                 IN"-o --prefer           (images only) Only allow certain formats (Default: %s)\n"
                 IN"-t --lang             Language settings. Used by a few getters to deliever localized data. Given in ISO 639-1 codes\n"
                 IN"-f --fuzzyness        Set treshold for level of Levenshtein algorithm.\n"
                 DEFAULT_FORMATS
                );

    glyr_message(-1,s,stdout,C_"\nAUTHOR: (C) Christopher Pahl - 2011, <sahib@online.de>\n%s\n",Gly_version());
    exit(EXIT_FAILURE);
}


static void usage(GlyQuery * s)
{
    glyr_message(-1,s,stderr,"Usage: glyrc [GETTER] (options)\n\nwhere [GETTER] must be one of:\n"
   		 C_B"\nGENERAL OPTIONS\n"C_
                 IN C_C"-f --from  <providerstring>\n"C_
		 IN IN"Use this to define what providers you want to use.\n"
		 IN IN"Every provider has a name and a key which is merely a shortcut for the name.\n"
		 IN IN"Specify all providers in a semicolon seperated list.\n"
		 IN IN"See the list down for a complete list of all providers.\n"
		 IN IN"Example:\n"
		 IN IN"  \"amazon;google\"\n" 
		 IN IN"  \"a;g\" - same, just with keys\n\n"
		 IN IN"You can also prepend each word with a '+' or a '-' ('+' is assumend without),\n"
		 IN IN"which will add or remove this provider from the list respectively.\n"
		 IN IN"Additionally you may use the predefined groups 'safe','unsafe','fast','slow','special'.\n\n"
		 IN IN"Example:\n"
		 IN IN"  \"+fast;-amazon\" which will enable last.fm and lyricswiki.\n\n"
                 IN C_C"-w --write <dir>\n"C_
                 IN C_C"-x --plugmax\n"C_
         	 IN IN"Maximum number of items a provider may deliever.\n"
         	 IN IN"Use this to scatter the searchresults, or set it\n"
         	 IN IN"to -1 if you don't care (=> default).\n\n"
                 IN C_C"-d --download\n"C_
         	 IN IN"For image getters only.\n"
         	 IN IN"If set to true images are also coviniently downloaded and returned.\n"
                 IN IN"otherwise, just the URL is returned for your own use. (specify -D)\n\n"
                 IN IN"Default to to 'true', 'false' would be a bit more searchengine like.\n"
                 IN C_C"-g --groups\n"C_
         	 IN IN"If set false (by -G), this will disable the grouping of providers.\n"
         	 IN IN"By default providers are grouped in categories like 'safe','unsafe','fast' etc., which\n"
                 IN IN"are queried in parallel, so the 'best' providers are queried first.\n"
                 IN IN"Disabling this behaviour will result in increasing speed, but as a result the searchresults\n"
		 IN IN"won't be sorted by quality, as it is normally the case.\n\n"
         	 IN IN"Use with care.\n\n"
		 IN C_C"-y --twincheck\n"C_
	         IN IN"Will check for duplicate items if given.\n"
   		 C_B"\nLIBCURL OPTIONS\n"C_
                 IN"-p --parallel <i>     Integer. Define the number of downloads that may be performed in parallel.\n"
                 IN"-r --redirects        Integer. Define the number of redirects that are allowed.\n"
                 IN"-m --timeout          Integer. Define the maximum number in seconds after which a download is cancelled.\n"
   		 C_B"\nPLUGIN SPECIFIC OPTIONS\n"C_
                 IN C_Y"-a --artist\n"C_
		 IN IN"The artist option is required for ALL getters.\n"
                 IN"-b --album\n"
		 IN IN"Required for the following getters:\n"
		 IN IN IN" - albumlist\n"
		 IN IN IN" - cover\n"
		 IN IN IN" - review\n"
		 IN IN IN" - tracklist\n"
		 IN IN"Optional for those:\n"
		 IN IN IN" - tags\n"
		 IN IN IN" - relations\n"
		 IN IN IN" - lyrics (might be used by a few providers)\n"
                 IN"-t --title\n"
		 IN IN"Set the songtitle.\n\n" 
		 IN IN"Required for:\n"
	         IN IN"- lyrics\n"
		 IN IN"Optional for:\n"
	         IN IN"- tags\n"
	         IN IN"- relations\n"
                 IN"-e --maxsize\n"
		 IN IN"The maximum size a cover may have.\n"
	         IN IN"As cover have mostly a 1:1 aspect ratio only one size is given with 'size'.\n"
		 IN IN"This has no effect if specified with 'photos'.\n"
                 IN"-i --minsize\n"
		 IN IN"Same as --maxsize, just for the minimum size.\n"
                 IN"-n --number\n"
		 IN IN"How many items to search for (at least 1)\n"
		 IN IN"This is mostly not the number of items actually returned then,\n"
	  	 IN IN"because libglyr is not able to find 300 songtexts of the same song,\n"
		 IN IN"or glyr filters duplicate items before returning.\n"
                 IN"-o --prefer           (images only) Only allow certain formats\n"
                 IN"-t --lang             Language settings. Used by a few getters to deliever localized data. Given in ISO 639-1 codes\n"
		 IN IN"The language used for providers with multilingual content.\n"
		 IN IN"It is given in ISO-639-1 codes, i.e 'de','en','fr' etc.\n"	
		 IN IN"List of providers recognizing this option:\n"
		 IN IN"  * cover/amazon (which amazon server to query)\n"
		 IN IN"  * cover/google (which google server to query)\n"
		 IN IN"  * ainfo/lastfm (the language the biography shall be in)\n"
		 IN IN"  * (this list should get longer in future releases)\n"
		 IN IN"(Use only these providers if you really want ONLY localized content)\n"
		 IN IN"By default all search results are in english.\n"
                 IN"-f --fuzzyness        Set treshold for level of Levenshtein algorithm.\n"
   		 C_B"\nMISC OPTIONS\n"C_
                 IN"-h --help\n"
		 IN IN"A shorter version of this text.\n"
                 IN"-H --usage\n"
		 IN IN"The text just jumping through your eyes in search for meaning.\n"
                 IN"-V --version\n"
		 IN IN"Print the version string, it's also at the end of this text.\n"
                 IN"-c --color            Enable colored output (Unix only)\n"
         	 IN IN"Maximum number of items a provider may deliever.\n"
                 IN IN"Use this to scatter the searchresults, or set it\n"
                 IN IN"to -1 if you don't care (=> default).\n"
                 IN"-v --verbosity\n"
		 IN IN"Set the verbosity level from 0-4, where:\n"
       		 IN IN"  0) nothing but fatal errors.\n"
                 IN IN"  1) warnings and important notes.\n"
                 IN IN"  2) normal, additional information what libglyr does.\n"
                 IN IN"  3) basic debug output.\n"
                 IN IN"  4) libcurl debug output.\n"
		 "\n"
                 IN"-u --update\n"
		 IN IN"If files are already present in the path given by --write (or '.' if none given),\n"
	 	 IN IN"no searching is performed by default. Use this flag to disable this behaviour.\n"
		 "\n\n\n"
		 IN"The boolean options -u,-c,-y,-g,-d  have an uppercase version which is inversing it's effect.(the long names are prepended by '--skip')\n"
                );
    


    glyr_message(-1,s,stderr, "A list of providers you can specify with --from folows:\n");
    glyr_message(-1,s,stderr,"\n"IN C_"cover:\n");
    list_provider_at_id(GET_COVER,13,s);
    glyr_message(-1,s,stderr,"\n"IN C_"lyrics:\n");
    list_provider_at_id(GET_LYRIC,13,s);
    glyr_message(-1,s,stderr,"\n"IN C_"photos:\n");
    list_provider_at_id(GET_PHOTO,13,s);
    glyr_message(-1,s,stderr,"\n"IN C_"review:\n");
    list_provider_at_id(GET_REVIEW,13,s);
    glyr_message(-1,s,stderr,"\n"IN C_"similiar:\n");
    list_provider_at_id(GET_SIMILIAR,13,s);
    glyr_message(-1,s,stderr,"\n"IN C_"ainfo:\n");
    list_provider_at_id(GET_AINFO,13,s);
    glyr_message(-1,s,stderr,"\n"IN C_"tracklist:\n");
    list_provider_at_id(GET_TRACKLIST,13,s);
    glyr_message(-1,s,stderr,"\n"IN C_"albumlist:\n");
    list_provider_at_id(GET_ALBUMLIST,13,s);
    glyr_message(-1,s,stderr,"\n"IN C_"tags\n");
    list_provider_at_id(GET_TAGS,13,s);
    glyr_message(-1,s,stderr,"\n"IN C_"relations\n");
    list_provider_at_id(GET_RELATIONS,13,s);


    print_version(s);
}

/* --------------------------------------------------------- */

static char ** parse_commandline_general(int argc, char * const * argv, GlyQuery * glyrs)
{
    int c;
    char ** write_arg = NULL;
    bool haswrite_arg = false;

    static struct option long_options[] =
    {
        {"from",          required_argument, 0, 'f'},
        {"write",         required_argument, 0, 'w'},
        {"parallel",      required_argument, 0, 'p'},
        {"redirects",     required_argument, 0, 'r'},
        {"timeout",       required_argument, 0, 'm'},
        {"plugmax",       required_argument, 0, 'x'},
        {"verbosity",     required_argument, 0, 'v'},
        {"update",        no_argument,       0, 'u'},
        {"skip-update",   no_argument,       0, 'U'},
        {"help",          no_argument,       0, 'h'},
        {"usage",         no_argument,       0, 'H'},
        {"version",       no_argument,       0, 'V'},
        {"color",         no_argument,       0, 'c'},
        {"skip-color",    no_argument,       0, 'C'},
        {"download",      no_argument,       0, 'd'},
        {"skip-download", no_argument,       0, 'D'},
        {"groups",        no_argument,       0, 'g'},
        {"skip-groups",   no_argument,       0, 'G'},
        {"twincheck",     no_argument,       0, 'y'},
        {"skip-twincheck",no_argument,       0, 'Y'},
        // -- plugin specific -- //
        {"artist",        required_argument, 0, 'a'},
        {"album",         required_argument, 0, 'b'},
        {"title",         required_argument, 0, 't'},
        {"minsize",       required_argument, 0, 'i'},
        {"maxsize",       required_argument, 0, 'e'},
        {"number",        required_argument, 0, 'n'},
        {"lang",          required_argument, 0, 'l'},
        {"fuzzyness",     required_argument, 0, 'z'},
        {"prefer",        required_argument, 0, 'r'},
        {0,               0,                 0, 'o'}
    };

    while (true)
    {
        int option_index = 0;
        c = getopt_long(argc, argv, "uUVhHcCyYdDgGf:w:p:r:m:x:v:a:b:t:i:e:n:l:z:o:",long_options, &option_index);

        // own error report
        opterr = 0;

        if (c == -1)
            break;

        switch (c)
        {
        case 'w':
        {
            size_t length = strlen(optarg);
            size_t offset = 0;
            size_t elemct = 0;

            char * c_arg = NULL;
            while( (c_arg = get_next_word(optarg,";",&offset,length)) != NULL)
            {
                const char * element = NULL;
                if(*c_arg && strcasecmp(c_arg,"stdout") && strcasecmp(c_arg,"stderr") && strcasecmp(c_arg,"null"))
                {
                    if(check_if_dir(c_arg) == false)
                    {
                        glyr_message(1,glyrs,stderr,C_R"*"C_" %s is not a valid directory.\n",c_arg);
                    }
                    else
                    {
                        element = dirname(c_arg);
                    }
                }
                else if(*c_arg)
                {
                    element = c_arg;
                }

                if(element)
                {
                    write_arg = realloc(write_arg, (elemct+2) * sizeof(char*));
                    write_arg[  elemct] = strdup(element);
                    write_arg[++elemct] = NULL;
                    haswrite_arg = true;
                }
                free(c_arg);
                c_arg=NULL;
            }
            if(elemct == 0)
            {
                glyr_message(1,glyrs,stderr,C_R"*"C_" you should give at least one valid dir to --w!\n");
                glyr_message(1,glyrs,stderr,C_R"*"C_" Will default to the current working directory.\n");
                write_arg = NULL;
            }
            break;
        }
        case 'u':
            update = true;
            break;
        case 'U':
            update = false;
            break;
        case 'y':
            GlyOpt_duplcheck(glyrs,true);
            break;
        case 'Y':
            GlyOpt_duplcheck(glyrs,false);
            break;
        case 'g':
            GlyOpt_groupedDL(glyrs,true);
            break;
        case 'G':
            GlyOpt_groupedDL(glyrs,false);
            break;
        case 'f':
            if(GlyOpt_from(glyrs,optarg) != GLYRE_OK)
                search_similiar_providers(optarg,glyrs);
            break;
        case 'v':
            GlyOpt_verbosity(glyrs,atoi(optarg));
            break;
        case 'p':
            GlyOpt_parallel(glyrs,atoi(optarg));
            break;
        case 'r':
            GlyOpt_redirects(glyrs,atoi(optarg));
            break;
        case 'm':
            GlyOpt_timeout(glyrs,atoi(optarg));
            break;
        case 'x':
            GlyOpt_plugmax(glyrs,atoi(optarg));
            break;
        case 'V':
            print_version(glyrs);
            break;
        case 'h':
            help_short(glyrs);
            break;
        case 'H':
            usage(glyrs);
            break;
        case 'c':
            GlyOpt_color(glyrs,true);
            break;
        case 'C':
            GlyOpt_color(glyrs,false);
            break;
        case 'a':
            GlyOpt_artist(glyrs,optarg);
            break;
        case 'b':
            GlyOpt_album(glyrs,optarg);
            break;
        case 't':
            GlyOpt_title(glyrs,optarg);
            break;
        case 'i':
            GlyOpt_cminsize(glyrs,atoi(optarg));
            break;
        case 'e':
            GlyOpt_cmaxsize(glyrs,atoi(optarg));
            break;
        case 'n':
            GlyOpt_number(glyrs,atoi(optarg));
            break;
        case 'd':
            GlyOpt_download(glyrs,true);
            break;
        case 'D':
            GlyOpt_download(glyrs,false);
            break;
        case 'l':
            GlyOpt_lang(glyrs,optarg);
            break;
        case 'z':
            GlyOpt_fuzzyness(glyrs,atoi(optarg));
            break;
        case 'o':
            GlyOpt_formats(glyrs,optarg);
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
    if(haswrite_arg) return write_arg;

    return NULL;
}

//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

char * correct_path(const char * path)
{
    char * r = NULL;
    if(path)
    {
        char * no_slash = strreplace(path,"/","+");
        if(no_slash)
        {
            char * no_spaces = strreplace(no_slash," ","+");
            if(no_spaces)
            {
                r = no_spaces;
            }
            free(no_slash);
            no_slash=NULL;
        }
    }
    return r;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_covers(GlyQuery * s, const char * save_dir, int i)
{
    char * good_artist = correct_path(s->artist);
    char * good_album  = correct_path(s->album );
    char * good_path   =  strdup_printf("%s/%s_%s_cover_%d.jpg",save_dir, good_artist,good_album,i);

    if(good_album)
        free(good_album);
    if(good_artist)
        free(good_artist);

    return good_path;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_lyrics(GlyQuery * s, const char * save_dir, int i)
{
    char * good_artist = correct_path(s->artist);
    char * good_title  = correct_path(s->title );
    char * good_path   =  strdup_printf("%s/%s_%s_lyrics_%d.txt",save_dir,good_artist,good_title,i);

    if(good_title)
        free(good_title);
    if(good_artist)
        free(good_artist);

    return good_path;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_photos(GlyQuery * s, const char * save_dir, int i)
{
    char * good_artist = correct_path(s->artist);
    char * good_path   = strdup_printf("%s/%s_photo_%d.jpg",save_dir,good_artist,i);

    if(good_artist)
        free(good_artist);

    return good_path;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_ainfo(GlyQuery * s, const char * save_dir, int i)
{
    char * good_artist = correct_path(s->artist);
    char * good_path   = strdup_printf("%s/%s_ainfo_%d.txt",save_dir,good_artist,i);

    if(good_artist)
        free(good_artist);

    return good_path;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_similiar(GlyQuery *s, const char * save_dir, int i)
{
    char * good_artist = correct_path(s->artist);
    char * good_path   = strdup_printf("%s/%s_similiar_%d.txt",save_dir, good_artist,i);

    if(good_artist)
        free(good_artist);

    return good_path;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_album_artist(GlyQuery *s, const char * save_dir, int i, const char * type)
{
    char * good_artist = correct_path(s->artist);
    char * good_album  = correct_path(s->album );
    char * good_path   = strdup_printf("%s/%s_%s_%s_%d.txt",save_dir,good_artist,good_album,type,i);

    if(good_artist)
        free(good_artist);
    if(good_album)
        free(good_album);

    return good_path;
}

/* --------------------------------------------------------- */
// ---------------------- MISC ----------------------------- //
/* --------------------------------------------------------- */

static char * path_review(GlyQuery *s, const char * save_dir, int i)
{
    return path_album_artist(s,save_dir,i,"review");
}

static char * path_tracklist(GlyQuery *s, const char * save_dir, int i)
{
    return path_album_artist(s,save_dir,i,"track");
}

static char * path_albumlist(GlyQuery *s, const char * save_dir, int i)
{
    char * good_artist = correct_path(s->artist);
    char * good_path   = strdup_printf("%s/%s_album_%d.txt",save_dir,good_artist,i);
    if(good_artist)
        free(good_artist);

    return good_path;
}

static char * path_tags(GlyQuery *s, const char * save_dir, int i)
{
    char * good_artist = correct_path(s->artist);
    char * good_path   = strdup_printf("%s/%s_tags_%d.txt",save_dir,s->artist,i);

    if(good_artist)
        free(good_artist);

    return good_path;
}

static char * path_relations(GlyQuery *s, const char * save_dir, int i)
{
    char * good_artist = correct_path(s->artist);
    char * good_path   = strdup_printf("%s/%s_tags_%d.txt",save_dir,s->artist,i);

    if(good_artist)
        free(good_artist);

    return good_path;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

char * get_path_by_type(GlyQuery * s, const char * sd, int iter)
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
    case GET_AINFO:
        m_path = path_ainfo(s,sd,iter);
        break;
    case GET_SIMILIAR:
        m_path = path_similiar(s,sd,iter);
        break;
    case GET_REVIEW:
        m_path = path_review(s,sd,iter);
        break;
    case GET_TRACKLIST:
        m_path = path_tracklist(s,sd,iter);
        break;
    case GET_ALBUMLIST:
        m_path = path_albumlist(s,sd,iter);
        break;
    case GET_TAGS:
        m_path = path_tags(s,sd,iter);
        break;
    case GET_RELATIONS:
        m_path = path_relations(s,sd,iter);
        break;
    }
    return m_path;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* -------------------------------------------------------- */

static void print_item(GlyQuery *s, GlyMemCache * cacheditem, int num)
{
    // GlyMemcache members
    // dsrc = Exact link to the location where the data came from
    // size = size in bytes
    // type = Type of data
    // data = actual data
    // (error) - Don't use this. Only internal use
    glyr_message(1,s,stderr,"------- ITEM #%d --------\n",num);
    glyr_message(1,s,stderr,"FROM: <%s>\n",cacheditem->dsrc);
    glyr_message(1,s,stderr,"SIZE: %d Bytes\n",(int)cacheditem->size);
    glyr_message(1,s,stderr,"TYPE: ");

    // Each cache identfies it's data by a constant
    switch(cacheditem->type)
    {
    case TYPE_COVER:
        glyr_message(1,s,stderr,"cover");
        break;
    case TYPE_COVER_PRI:
        glyr_message(1,s,stderr,"cover (frontside)");
        break;
    case TYPE_COVER_SEC:
        glyr_message(1,s,stderr,"cover (backside or inlet)");
        break;
    case TYPE_LYRICS:
        glyr_message(1,s,stderr,"songtext");
        break;
    case TYPE_PHOTOS:
        glyr_message(1,s,stderr,"band photo");
        break;
    case TYPE_REVIEW:
        glyr_message(1,s,stderr,"albumreview");
        break;
    case TYPE_AINFO:
        glyr_message(1,s,stderr,"artistbio");
        break;
    case TYPE_SIMILIAR:
        glyr_message(1,s,stderr,"similiar artist");
        break;
    case TYPE_TRACK:
        glyr_message(1,s,stderr,"trackname [%d:%02d]",cacheditem->duration/60,cacheditem->duration%60);
        break;
    case TYPE_ALBUMLIST:
        glyr_message(1,s,stderr,"albumname");
        break;
    case TYPE_TAGS:
        glyr_message(1,s,stderr,"some tag");
        break;
    case TYPE_TAG_ARTIST:
        glyr_message(1,s,stderr,"artisttag");
        break;
    case TYPE_TAG_ALBUM:
        glyr_message(1,s,stderr,"albumtag");
        break;
    case TYPE_TAG_TITLE:
        glyr_message(1,s,stderr,"titletag");
        break;
    case TYPE_RELATION:
        glyr_message(1,s,stderr,"relation");
        break;
    case TYPE_NOIDEA:
    default:
        glyr_message(1,s,stderr,"No idea...?");
    }

    // Print the actual data.
    // This might have funny results if using cover/photos
    if(!cacheditem->is_image)
        glyr_message(1,s,stderr,"\nDATA:\n%s",cacheditem->data);
    else
        glyr_message(1,s,stderr,"\nDATA: <not printable>");

    glyr_message(1,s,stderr,"\n");
}

/* --------------------------------------------------------- */

static enum GLYR_ERROR cb(GlyMemCache * c, GlyQuery * s)
{
    // This is just to demonstrate the callback option.
    // Put anything in here that you want to be executed when
    // a cache is 'ready' (i.e. ready for return)
    // See the glyr_set_dl_callback for more info
    // a custom pointer is in s->callback.user_pointer
    int * i = s->callback.user_pointer;
    print_item(s,c,(*i = *i + 1));
    return GLYRE_OK;
}

/* --------------------------------------------------------- */

int main(int argc, char * argv[])
{
    int result = EXIT_SUCCESS;

    /* Warn on  crash */
    signal(SIGSEGV, sig_handler);
    signal(SIGINT,  sig_handler);

    if(argc >= 3)
    {
        // make sure to init everything and destroy again
        Gly_init();
        atexit(Gly_cleanup);

        GlyQuery my_query;
        // glyr's control struct
        Gly_init_query(&my_query);
        GlyOpt_verbosity(&my_query,2);

        /*
        // Left as example
        GlyOpt_call_direct_use(&my_query, true);
        GlyOpt_call_direct_provider(&my_query, "a");
        GlyOpt_call_direct_url(&my_query, "http://free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId=AKIAJ6NEA642OU3FM24Q&Operation=ItemSearch&SearchIndex=Music&ResponseGroup=Images&Keywords=cher+believe");
        */

        // Set the type..
        if(!set_get_type(&my_query, argv[1]))
        {
            Gly_destroy_query( &my_query );
            return EXIT_FAILURE;
        }

        char ** write_arg = parse_commandline_general(argc-1, argv+1, &my_query);
        if(write_arg == NULL)
        {
            write_arg = malloc(2 * sizeof(char*));
            write_arg[0] = strdup(default_path);
            write_arg[1] = NULL;
        }

        if(my_query.type == GET_AINFO)
            my_query.number *= 2;

        // Check if files do already exist
        bool file_exist = false;

        int iter = 0;
        for(iter = 0; iter < my_query.number; iter++)
        {
            size_t j = 0;
            for(j = 0; write_arg[j]; j++)
            {
                char * path = get_path_by_type(&my_query, write_arg[j], iter);
                if(path)
                {
                    if(!update && (file_exist = !access(path,R_OK) ))
                    {
                        free(path);
                        break;
                    }
                    free(path);
                    path = NULL;
                }
            }
        }

        // Set (example) callback
        int item_counter = 0;
        GlyOpt_dlcallback(&my_query, cb, &item_counter);
        if(my_query.type != GET_UNSURE)
        {
            if(!file_exist)
            {
                // Now download everything
                enum GLYR_ERROR get_error = GLYRE_OK;
                GlyCacheList * my_list= Gly_get(&my_query, &get_error);
                if(my_list)
                {
                    GlyPlugin * table_copy = Gly_get_provider_by_id(GET_UNSURE);
                    if(get_error == GLYRE_OK)
                    {
                        size_t i = 0;
                        for(i = 0; i < my_list->size && my_list->list[i]; i++)
                        {
                            size_t j = 0;
                            for(j = 0; write_arg[j]; j++)
                            {
                                char * path = get_path_by_type(&my_query,write_arg[j],i);
                                if(path != NULL)
                                {
				    if(write_arg[j] && strcmp(write_arg[j],"stdout") && strcmp(write_arg[j],"stderr"))
                		      glyr_message(1,&my_query,stdout,C_"- Writing %s to %s\n", table_copy[my_query.type].name,path);
				    else
				      glyr_message(2,&my_query,stderr,"------------\n");

                                    if(Gly_write(&my_query,my_list->list[i],path) == -1)
                                    {
                                        result = EXIT_FAILURE;
                                    }
                                    free(path);
                                    path = NULL;
                                }
                            }
                        }
                    }

                    // Free all downloaded buffers
                    Gly_free_list(my_list);
                    free(table_copy);
                }
                else if(get_error != GLYRE_OK)
                {
                    glyr_message(1,&my_query,stderr,"E: %s\n",Gly_strerror(get_error));
                }
            }
            else
            {
                glyr_message(1,&my_query,stderr,C_B"*"C_" File(s) already exist. Use -u to update.\n");
            }

            size_t x = 0;
            for( x = 0; write_arg[x]; x++)
            {
                free(write_arg[x]);
                write_arg[x] = NULL;
            }
            free(write_arg);
            write_arg = NULL;

            // Clean memory alloc'd by settings
            Gly_destroy_query( &my_query);
        }
    }
    else if(argc >= 2 && !strcmp(argv[1],"-V"))
    {
        print_version(NULL);
    }
    else if(argc >= 2 && (!strcmp(argv[1],"-C") || !strcmp(argv[1],"-CH")))
    {
        GlyQuery tmp;
        tmp.color_output = false;
        usage(&tmp);
    }
    else if(argc >= 2 && (!strcmp(argv[1],"-h") || !strcmp(argv[1],"-ch") || !strcmp(argv[1],"-hc")))
    {
        help_short(NULL);
    }
    else
    {
        GlyQuery tmp;
        tmp.color_output = false;
        help_short(&tmp);
    }

    // byebye
    return result;
}

//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */
