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

static void print_version(void)
{
    glyr_message(-1,NULL,stdout, C_G"%s\n\n"C_,glyr_version());
    glyr_message(-1,NULL,stderr, C_"This is still beta software, expect quite a lot bugs.\n");
    glyr_message(-1,NULL,stderr, C_"Email bugs to <sahib@online.de> or use the bugtracker\n"
                 C_"at https://github.com/sahib/glyr/issues - Thank you! \n");
    exit(0);
}

//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

const char * next_group(plugin_t * table, int s)
{
	int i;
	for(i = s; table[i].name; i++)
	{
		if(!table[i].key)
		    return table[i].name;
	}
	return NULL;
}

#define _S "\t"
static void list_provider_at_id(int id, int min_align)
{
    plugin_t * cp = glyr_get_provider_by_id(id);
    const char * c_group = next_group(cp,0);

    if(cp != NULL)
    {
	size_t i = 0;
	size_t max_align = min_align;

	for(i = 0; cp[i].name; i++)
	{
		if(cp[i].key)
		{
			// align text. Help texts are sooo boring :-)
			size_t x = 0;
			size_t name_len = strlen(cp[i].name);
			if(name_len >= max_align)
			{
				max_align = name_len + 1;
			}

			size_t align = max_align - name_len;

			glyr_message(-1,NULL,stdout,_S"- "C_G"%s"C_,cp[i].name);
			for(x = 0; x < align; x++)
			{
				glyr_message(-1,NULL,stdout," ");
			}
			glyr_message(-1,NULL,stdout,"["C_R"%s"C_"]"C_,cp[i].key);

			if(id != -1)
			{
				glyr_message(-1,NULL,stdout," in group "C_Y"all"C_","C_Y"%s"C_,c_group);
			}
			glyr_message(-1,NULL,stdout,"\n");
		}
		else
		{
			c_group = next_group(cp,i+1);
		}
	}
	free(cp);
    }
}

static void usage(void)
{
    glyr_message(-1,NULL,stdout,C_B"USAGE:\n"C_ _S"glyrc "C_Y"GETTER"C_" [OPTIONS...]\n\n");
    glyr_message(-1,NULL,stdout,"glyrc downloads variouse sorts of musicrelated metadata.\n");
    glyr_message(-1,NULL,stdout,C_Y"GETTER"C_" is the type of metadata to download, it must be one of: \n");
    
    list_provider_at_id(-1, 7);
 
#define GET_C C_G
#define OPT_C C_C
#define OPT_A OPT_C"\n    "
    glyr_message(-1,NULL,stdout,"\nThe getter has to be always the very first argument given, and may require other arguments\n");
    glyr_message(-1,NULL,stdout,"A short note to the terminology: a provider is a source glyrc may download data from.\n");
    glyr_message(-1,NULL,stdout,"A group is a collection of providers, each getter has at least the groups all,safe and unsafe,\n");
    glyr_message(-1,NULL,stdout,"Please note that providers and groups may differ heavily from getter to getter!\n");
    glyr_message(-1,NULL,stdout,"you can modify glyrc's providerlist by passing the names (or their shortcuts) to --from.\n");
    glyr_message(-1,NULL,stdout,"To find out what getter needs what argument see below: ("OPT_C"-n"C_" is the max number of items)\n");
    glyr_message(-1,NULL,stdout,C_B"\nCOVER\n"C_);
    glyr_message(-1,NULL,stdout,_S"Download "OPT_C"-n"C_" albumart images of the artist and album specified with "OPT_C"-a and -b\n"C_);
    glyr_message(-1,NULL,stdout,_S"The filetype of the image is unspecified (as libglyr often could only vaguely guess)\n");
    glyr_message(-1,NULL,stdout,_S"and may be determinded by its header; glyrc saves it as '.img in all cases.'\n");
    glyr_message(-1,NULL,stdout,_S"Currently "OPT_C"--from"C_" takes the following strings here:\n\n");
    list_provider_at_id(GET_COVER,12);
    glyr_message(-1,NULL,stdout,C_B"LYRICS\n"C_);
    glyr_message(-1,NULL,stdout,_S"Download "OPT_C"-n"C_" lyrics of the artist and title specified with "OPT_C"-a"C_" and "OPT_C"-t\n"C_);
    glyr_message(-1,NULL,stdout,_S"The album ("OPT_C"-b"C_") is optional and may be used by some plugins.\n");
    glyr_message(-1,NULL,stdout,_S"Currently "OPT_C"--from"C_" takes the following strings here:\n\n");
    list_provider_at_id(GET_LYRIC,13);
    glyr_message(-1,NULL,stdout,C_B"PHOTOS\n"C_);
    glyr_message(-1,NULL,stdout,_S"Download "OPT_C"-n"C_" photos that are related to the artist given by "OPT_C"-a"C_".\n");
    glyr_message(-1,NULL,stdout,_S"Currently "OPT_C"--from"C_" takes the following strings here:\n\n");
    list_provider_at_id(GET_PHOTO,12);
    glyr_message(-1,NULL,stdout,C_B"AINFO\n"C_);
    glyr_message(-1,NULL,stdout,_S"Download "OPT_C"-n"C_" artist descriptions of the artist given by "OPT_C"-a"C_".\n");
    glyr_message(-1,NULL,stdout,_S"Currently "OPT_C"--from"C_" takes the following strings here:\n\n");
    list_provider_at_id(GET_AINFO,12);
    glyr_message(-1,NULL,stdout,C_B"\nGENERAL OPTIONS\n"C_);
    glyr_message(-1,NULL,stdout,OPT_A"-f --from <prov>\n"C_);
    glyr_message(-1,NULL,stdout,_S"Set the sources (providers) you want to query.\n");
    glyr_message(-1,NULL,stdout,_S"The string you have to provide cotains the names of the providers or a group\n");
    glyr_message(-1,NULL,stdout,_S"(or their shortcuts), seperated by a \"%s\" and prepended by a + or -,\n", FROM_ARGUMENT_DELIM);
    glyr_message(-1,NULL,stdout,_S"which adds or deletes this source to/from the current state.\n");
    glyr_message(-1,NULL,stdout,_S"An example would be: \"+all%s-special%s+d\"\n",FROM_ARGUMENT_DELIM,FROM_ARGUMENT_DELIM );
    glyr_message(-1,NULL,stdout,_S _S "+all     : adds everything.\n");
    glyr_message(-1,NULL,stdout,_S _S "-special : subtract the members of group 'special'.\n");
    glyr_message(-1,NULL,stdout,_S _S "+d       : add the provider 'd' (discogs, see above)\n");
    glyr_message(-1,NULL,stdout,OPT_A"-n --number <int>\n"C_);
    glyr_message(-1,NULL,stdout,_S"Maximum number of items a getter may download.\n");
    glyr_message(-1,NULL,stdout,_S"Default is %d.\n", DEFAULT_NUMBER);
    glyr_message(-1,NULL,stdout,OPT_A"-x --plugmax <int>\n"C_);
    glyr_message(-1,NULL,stdout,_S"Maximum number of items a plugin may download.\n");
    glyr_message(-1,NULL,stdout,_S"Default is %d.\n", DEFAULT_PLUGMAX);
    glyr_message(-1,NULL,stdout,OPT_A"-u --update\n"C_);
    glyr_message(-1,NULL,stdout,_S"Update file even if already present.\n");
    glyr_message(-1,NULL,stdout,_S"Default is %s.\n",update ? "true" : "false");
    glyr_message(-1,NULL,stdout,OPT_A"-d --nodownload\n"C_);
    glyr_message(-1,NULL,stdout,_S"Do not download final result, only print URL.\n");
    glyr_message(-1,NULL,stdout,_S"This only works for images, as lyrics and ainfo do not neccesarely have a concrete URL.\n");
    glyr_message(-1,NULL,stdout,_S"Use this to turn glyrc into some sort of music-metadate search engine.\n");
    glyr_message(-1,NULL,stdout,_S"Default is %s.\n",DEFAULT_DOWNLOAD ? "true" : "false");
    glyr_message(-1,NULL,stdout,OPT_A"-w --write <dir>\n"C_);
    glyr_message(-1,NULL,stdout,_S"Write all files to the directory <dir>\n");
    glyr_message(-1,NULL,stdout,_S"The filenames itself is determined by the artist,album, title depending on <GET>\n");
    glyr_message(-1,NULL,stdout,_S"The special value \"stdout\" will print the data directly to stdout, \"stderr\" to stderr\n"_S"and \"null\" will print nothing.\n");
    glyr_message(-1,NULL,stdout,C_B"\nLIBCURL OPTIONS\n"C_);
    glyr_message(-1,NULL,stdout,OPT_A"-p --parallel <int>\n"C_);
    glyr_message(-1,NULL,stdout,_S"Download max. <int> files in parallel if there's more than one to download.\n");
    glyr_message(-1,NULL,stdout,_S"This is useful for downloading photos in line; Default is %d.\n", DEFAULT_PARALLEL);
    glyr_message(-1,NULL,stdout,OPT_A"-r --redirects <int>\n"C_);
    glyr_message(-1,NULL,stdout,_S"Allow max. <int> redirects. This is only used for Amazon and you should never need to use it.\n");
    glyr_message(-1,NULL,stdout,_S"Default is %d.\n", DEFAULT_REDIRECTS);
    glyr_message(-1,NULL,stdout,OPT_A"-m --timeout <int>\n"C_);
    glyr_message(-1,NULL,stdout,_S"Maximum number of <int> seconds to wait before cancelling a download.\n");
    glyr_message(-1,NULL,stdout,_S"Default is %d.\n", DEFAULT_TIMEOUT);
    glyr_message(-1,NULL,stdout,C_B"\nMISC OPTIONS\n"C_);
    glyr_message(-1,NULL,stdout,OPT_A"-V --version\n"C_);
    glyr_message(-1,NULL,stdout,_S"Print version string and exit.\n");
    glyr_message(-1,NULL,stdout,OPT_A"-h --help\n"C_);
    glyr_message(-1,NULL,stdout,_S"Print this help and exit.\n");
    glyr_message(-1,NULL,stdout,OPT_A"-c --color\n"C_);
    glyr_message(-1,NULL,stdout,_S"Enables colored console output (Unix only).\n");
    glyr_message(-1,NULL,stdout,_S"Default is %s.\n", PRT_COLOR ? "true" : "false");
    glyr_message(-1,NULL,stdout,OPT_A"-v --verbosity <int>\n"C_);
    glyr_message(-1,NULL,stdout,_S"Level of verbosity:\n");
    glyr_message(-1,NULL,stdout,_S _S"-v0: Print nothing but fatal errors.\n");
    glyr_message(-1,NULL,stdout,_S _S"-v1: Print only basic information.\n");
    glyr_message(-1,NULL,stdout,_S _S"-v2: Print informative output. (default)\n");
    glyr_message(-1,NULL,stdout,_S _S"-v3: Enable debugging messages.\n");
    glyr_message(-1,NULL,stdout,_S _S"-v4: Enable libcurl-debugging messages.\n");
    glyr_message(-1,NULL,stdout,C_B"\nPLUGIN OPTIONS\n"C_);
    glyr_message(-1,NULL,stdout,OPT_A"-a --artist <string>; -b --album <string>; -t --title <string>\n"C_);
    glyr_message(-1,NULL,stdout,_S"Depending on the getter you have to provide information on what to search.\n");
    glyr_message(-1,NULL,stdout,_S"Please refer to the getter description to find what argument is needed for what getter.\n");
    glyr_message(-1,NULL,stdout,OPT_A"-i --minsize\n"C_);
    glyr_message(-1,NULL,stdout,_S"For cover only: The minimum size a coverimage may have\n");
    glyr_message(-1,NULL,stdout,_S"A value of -1 will disable the sizecheck.\n");
    glyr_message(-1,NULL,stdout,_S"Note that this value is only a 'suggestion' for libglyr,\n");
    glyr_message(-1,NULL,stdout,_S"but it should work in 95%% of all cases.\n");
    glyr_message(-1,NULL,stdout,_S"Default is %d.\n",DEFAULT_CMINSIZE);
    glyr_message(-1,NULL,stdout,OPT_A"-e --maxsize\n"C_);
    glyr_message(-1,NULL,stdout,_S"For cover only: The maximum size a coverimage may have (see above)\n");
    glyr_message(-1,NULL,stdout,_S"A value of -1 will disable the sizecheck.\n");
    glyr_message(-1,NULL,stdout,_S"Default is %d.\n",DEFAULT_CMAXSIZE);
    glyr_message(-1,NULL,stdout,OPT_A"-l --lang <l>\n"C_);
    glyr_message(-1,NULL,stdout,_S"For cover:\n"_S _S"decides which amazon / google server is queried,\n"_S _S"what may have impact on search results.\n\n");
    glyr_message(-1,NULL,stdout,_S"For ainfo's last.fm plugin:\n"_S _S"Decide in which language results are returned.\n\n");
    glyr_message(-1,NULL,stdout,_S"The language is given by ISO 639-1 codes;\n");
    glyr_message(-1,NULL,stdout,_S"Examples are: us,ca,uk,fr,de,jp\n");
    glyr_message(-1,NULL,stdout,C_B"\nEXAMPLES\n"C_);
    glyr_message(-1,NULL,stdout,_S"glyrc cover -a Equilibrium -b \"Turis Fratyr\"\n");
    glyr_message(-1,NULL,stdout,_S _S"# Download the cover art of the album 'Turis Fratyr' by the band equilibrium,\n");
    glyr_message(-1,NULL,stdout,_S _S"# save it to Equilibrium_Turis+Fratyr_0.img\n");
    glyr_message(-1,NULL,stdout,_S"glyrc cover -a Equilibrium -b Rekreatur -n 5 -i 100 -e 250 --from \"amazon\"\n");
    glyr_message(-1,NULL,stdout,_S _S"# Download 5x (different) images of 'Rekreatur',\n");
    glyr_message(-1,NULL,stdout,_S _S"# with the dimensions in between 100x100 and 250x250 from amazon\n");
    glyr_message(-1,NULL,stdout,_S"glyrc lyrics -a Equilibrium -t \"Blut im Auge\"\n");
    glyr_message(-1,NULL,stdout,_S _S"# Download lyrics of the Song \"Blut im Auge\" by Equilibrium,\n");
    glyr_message(-1,NULL,stdout,_S _S"# show it in stdout, and write it to Equilibrium_Blut+im+Auge_0.lyrics\n");
    glyr_message(-1,NULL,stdout,C_B"\nFILES\n"C_);
    glyr_message(-1,NULL,stdout,_S"Everything is stored in a file with the pattern $save_dir/$artist_($album|$title)_suffix.type\n");
    glyr_message(-1,NULL,stdout,_S"Spaces in artist/album/title are escaped with a '+'.\n");
    glyr_message(-1,NULL,stdout,_S _S"$artist_$album_$num.img\n");
    glyr_message(-1,NULL,stdout,_S _S"$artist_$title_$num.lyrics\n");
    glyr_message(-1,NULL,stdout,_S _S"$artist_photos_$num.img\n");
    glyr_message(-1,NULL,stdout,_S _S"$artist_artist_$num.ainfo\n");
    glyr_message(-1,NULL,stdout,_S"\n");
   
    print_version();
    exit(0);
}
#undef S
#undef OPT_C
#undef GET_C

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

	list_provider_at_id(-1,7);

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
                            glyr_message(2,s,stderr,"Did you mean");
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
	
	glyr_message(2,s,stderr,"Must be one of:\n");
	list_provider_at_id(s->type,15);
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
        {"lang",      required_argument, 0, 'l'},
        {0,           0,                 0,  0 }
    };

    while (true)
    {
        int option_index = 0;
        c = getopt_long(argc, argv, "uVhcdf:w:p:r:m:x:v:a:b:t:i:e:n:l:",long_options, &option_index);

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

        case 'V':
            print_version();
            break;

        case 'h':
            usage();
            break;
	case 'c':
	    glyr_setopt(glyrs,GLYRO_COLOR,(PRT_COLOR)?false:true);
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
    char * good_artist = correct_path(s->artist);
    char * good_album  = correct_path(s->album );
    char * good_path   =  strdup_printf("%s/%s_%s_%d.img",save_dir, good_artist,good_album,i);
    
    if(good_album)
	free(good_album);
    if(good_artist)
	free(good_artist);

    return good_path;
}

static void handle_covers(memCache_t * cache, glyr_settings_t * s)
{
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_lyrics(glyr_settings_t * s, const char * save_dir, int i)
{
    char * good_artist = correct_path(s->artist);
    char * good_title  = correct_path(s->title );
    char * good_path   =  strdup_printf("%s/%s_%s_%d.lyrics",save_dir,good_artist,good_title,i);
    
    if(good_title)
	free(good_title);
    if(good_artist)
	free(good_artist);

    return good_path;
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
    char * good_artist = correct_path(s->artist);
    char * good_path   = strdup_printf("%s/%s_photo_%d.img",save_dir,good_artist,i);

    if(good_artist)
	free(good_artist);
    
    return good_path;
}

static void handle_photos(memCache_t* cache, glyr_settings_t * s)
{
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_ainfo(glyr_settings_t * s, const char * save_dir, int i)
{
    char * good_artist = correct_path(s->artist);
    char * good_path   = strdup_printf("%s_artist_%d.ainfo\n",good_artist,i);

    if(good_artist)
	free(good_artist);
    
    return good_path;
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
    char * good_artist = correct_path(s->artist);
    char * good_path   = strdup_printf("%s/%s.book_%d",save_dir, good_artist,i);
	
    if(good_artist)
	free(good_artist);

    return good_path;
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
