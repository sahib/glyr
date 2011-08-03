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

/* Somehow needed for g_vasprintf */
#include <glib/gprintf.h>

/* All you need from libglyr */
#include "../lib/glyr.h"

/* Compile information, you do not have this header */
#include "../lib/config.h"

/* Globals */
const gchar * exec_on_call = NULL;
const gchar ** write_arg   = NULL;

int message(int verbosity, GlyQuery * s, FILE * stream, const char * fmt, ...)
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
//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static void print_version(GlyQuery * s)
{
    message(-1,s,stdout, "%s\n\n",Gly_version());
    message(-1,s,stderr, "This is still beta software, expect quite a lot bugs.\n");
    message(-1,s,stderr, "Email bugs to <sahib@online.de> or use the bugtracker\n"
                 "at https://github.com/sahib/glyr/issues - Thank you! \n");

    exit(0);
}


//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

void help_short(GlyQuery * s)
{
    message(-1,s,stderr,"Usage: glyrc [GETTER] (options)\n\nwhere [GETTER] must be one of:\n");

// spacer
#define IN "    "
    message(-1,s,stderr,"\n\nOPTIONS:\n"
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
                 IN"-V --version          Print the version string.\n"
                 IN"-d --download         Download Images.\n"
                 IN"-D --skip-download    Don't download images, but return the URLs to them (act like a search engine)\n"
                 IN"-g --groups           Enable grouped download (Slower but more accurate, as quality > speed)\n"
                 IN"-G --skip-groups      Query all providers at once. (Faster but may deliever weird results)\n"
                 IN"-a --artist           Artist name (Used by all plugins)\n"
                 IN"-b --album            Album name (Used by cover,review,lyrics)\n"
                 IN"-t --title            Songname (used mainly by lyrics)\n"
                 IN"-e --maxsize          (cover only) The maximum size a cover may have.\n"
                 IN"-i --minsize          (cover only) The minimum size a cover may have.\n"
                 IN"-n --number           Download max. <n> items. Amount of actual downloaded items may be less.\n"
                 IN"-t --lang             Language settings. Used by a few getters to deliever localized data. Given in ISO 639-1 codes\n"
                 IN"-f --fuzzyness        Set treshold for level of Levenshtein algorithm.\n"
                 IN"-j --callback         Set a bash command to be executed when a item is finished downloading;\n"
                 IN"-k --proxy	      Set the proxy to use in the form of [protocol://][user:pass@]yourproxy.domain[:port]\n"
                 IN"                      The special string <path> is expanded with the actual path to the data.\n"
                );

    message(-1,s,stdout,"\nAUTHOR: (C) Christopher Pahl - 2011, <sahib@online.de>\n%s\n",Gly_version());
    exit(EXIT_FAILURE);
#undef IN
}

/* --------------------------------------------------------- */

static void parse_commandline_general(int argc, char * const * argv, GlyQuery * glyrs, gchar ** write_to)
{
	static struct option long_options[] =
	{
		{"from",          required_argument, 0, 'f'},
		{"write",         required_argument, 0, 'w'},
		{"parallel",      required_argument, 0, 'p'},
		{"redirects",     required_argument, 0, 'r'},
		{"timeout",       required_argument, 0, 'm'},
		{"plugmax",       required_argument, 0, 'x'},
		{"verbosity",     required_argument, 0, 'v'},
		{"help",          no_argument,       0, 'h'},
		{"usage",         no_argument,       0, 'H'},
		{"version",       no_argument,       0, 'V'},
		{"download",      no_argument,       0, 'd'},
		{"no-download",   no_argument,       0, 'D'},
		{"groups",        no_argument,       0, 'g'},
		{"no-groups",     no_argument,       0, 'G'},
		// ---------- plugin specific ------------ //
		{"artist",        required_argument, 0, 'a'},
		{"album",         required_argument, 0, 'b'},
		{"title",         required_argument, 0, 't'},
		{"minsize",       required_argument, 0, 'i'},
		{"maxsize",       required_argument, 0, 'e'},
		{"number",        required_argument, 0, 'n'},
		{"lang",          required_argument, 0, 'l'},
		{"fuzzyness",     required_argument, 0, 'z'},
		{"prefer",        required_argument, 0, 'o'},
		{"callback",	  required_argument, 0, 'j'},
		{"proxy",	  required_argument, 0, 'k'},
		{0,               0,                 0, '0'}
	};

	while (true)
	{
		int c;
		int option_index = 0;
		if((c = getopt_long_only(argc, argv, "VhHdDgGf:w:p:r:m:x:v:a:b:t:i:e:n:l:z:o:j:k:",long_options, &option_index)) == -1)
		{
			break;
		}

		/* Disable getopt's error messages */ 
		opterr = 0;

		switch (c)
		{
			case 'w':
				{
					gsize opt_len = strlen(optarg);
					if(g_ascii_strncasecmp(optarg,"stdout",opt_len) == 0 &&
					   g_ascii_strncasecmp(optarg,"stderr",opt_len) == 0 &&
					   g_ascii_strncasecmp(optarg,"null",  opt_len) == 0 &&
					   g_file_test(optarg,G_FILE_TEST_IS_DIR | G_FILE_TEST_EXISTS) == TRUE)
					{
						*write_to = optarg;
					}
					else
					{
						g_printerr("'%s' does not seem to be an valid directory!\n",optarg);	
					}
					break;
				}
			case 'g':
				GlyOpt_groupedDL(glyrs,true);
				break;
			case 'G':
				GlyOpt_groupedDL(glyrs,false);
				break;
			case 'f':
				GlyOpt_from(glyrs,optarg);
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
			case 'j':
				exec_on_call = optarg;
				break;
			case 'k':
				GlyOpt_proxy(glyrs,optarg);
				break;
			case '?':
				break;
		}
	}
}

//* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static void replace_char(gchar * string, gchar a, gchar b, gsize len)
{
	if(string != NULL)
	{
		gsize str_len = (len == 0) ? strlen(string) : len;
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

char * correct_path(const char * path)
{
	if(path != NULL)
	{
		gchar * no_slash = g_strdup(path);
		replace_char(no_slash,'/','+',0);

		if(no_slash)
		{
			replace_char(no_slash,' ','+',0);
		}
		return no_slash;
	}
	return NULL;
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* --------------------------------------------------------- */

static char * path_covers(GlyQuery * s, const char * save_dir, int i)
{
	char * good_artist = correct_path(s->artist);
	char * good_album  = correct_path(s->album );
	char * good_path   = g_strdup_printf("%s/%s_%s_cover_%d.jpg",save_dir, good_artist,good_album,i);

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
	char * good_path   = g_strdup_printf("%s/%s_%s_lyrics_%d.txt",save_dir,good_artist,good_title,i);

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
	char * good_path   = g_strdup_printf("%s/%s_photo_%d.jpg",save_dir,good_artist,i);

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
	char * good_path   = g_strdup_printf("%s/%s_ainfo_%d.txt",save_dir,good_artist,i);

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
	char * good_path   = g_strdup_printf("%s/%s_similiar_%d.txt",save_dir, good_artist,i);

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
	char * good_path   = g_strdup_printf("%s/%s_%s_%s_%d.txt",save_dir,good_artist,good_album,type,i);

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

/* --------------------------------------------------------- */

static char * path_tracklist(GlyQuery *s, const char * save_dir, int i)
{
	return path_album_artist(s,save_dir,i,"tracktitle");
}

/* --------------------------------------------------------- */

static char * path_albumlist(GlyQuery *s, const char * save_dir, int i)
{
	char * good_artist = correct_path(s->artist);
	char * good_path   = g_strdup_printf("%s/%s_albumtitle_%d.txt",save_dir,good_artist,i);
	if(good_artist)
		free(good_artist);

	return good_path;
}

/* --------------------------------------------------------- */

static char * path_tags(GlyQuery *s, const char * save_dir, int i)
{
	char * good_artist = correct_path(s->artist);
	char * good_path   = g_strdup_printf("%s/%s_tag_%d.txt",save_dir,s->artist,i);

	if(good_artist)
		free(good_artist);

	return good_path;
}

/* --------------------------------------------------------- */

static char * path_relations(GlyQuery *s, const char * save_dir, int i)
{
	char * good_artist = correct_path(s->artist);
	char * good_path   = g_strdup_printf("%s/%s_url_%d.txt",save_dir,s->artist,i);

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
		case GET_COVERART:
			m_path = path_covers(s,sd,iter);
			break;
		case GET_LYRICS:
			m_path = path_lyrics(s,sd,iter);
			break;
		case GET_ARTIST_PHOTOS:
			m_path = path_photos(s,sd,iter);
			break;
		case GET_ARTISTBIO:
			m_path = path_ainfo(s,sd,iter);
			break;
		case GET_SIMILIAR_ARTISTS:
			m_path = path_similiar(s,sd,iter);
			break;
		case GET_SIMILIAR_SONGS:
			m_path = path_similiar(s,sd,iter);
			break;
		case GET_ALBUM_REVIEW:
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
		case GET_UNSURE:
			message(-1,NULL,stderr,"# getPath(): Unknown type, Problem?\n");
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
	message(1,s,stderr,"\n------- ITEM #%d --------\n",num);
	Gly_printitem(s,cacheditem);
	message(1,s,stderr,"\n------------------------\n");
	message(1,s,stderr,"\n");
}

/* --------------------------------------------------------- */
// --------------------------------------------------------- //
/* -------------------------------------------------------- */

static enum GLYR_ERROR callback(GlyMemCache * c, GlyQuery * s)
{
	// This is just to demonstrate the callback option.
	// Put anything in here that you want to be executed when
	// a cache is 'ready' (i.e. ready for return)
	// See the glyr_set_dl_callback for more info
	// a custom pointer is in s->callback.user_pointer
	int * current = s->callback.user_pointer;

	// Text Represantation of this item
	print_item(s,c,(*current));

	/* write out 'live' */
	if(write_arg)
	{
		size_t opt = 0;
		for(opt = 0; write_arg[opt]; opt++)
		{
			gchar * path = get_path_by_type(s,write_arg[opt],*current);
			if(path != NULL)
			{
				message(1,s,stderr,"- Writing '%d' to %s\n",c->type,path);

				if(Gly_write(c,path) == -1)
				{
					message(1,s,stderr,"(!!) glyrc: writing data to <%s> failed.\n",path);
				}
			}

			/* call the program if any specified */
			if(exec_on_call != NULL)
			{
				char * replace_path = g_strdup(exec_on_call);
				if(path != NULL)
				{
					gchar ** path_splitv = g_strsplit(path,"<path>",0);
					replace_path = g_strjoinv(path,path_splitv);
					g_strfreev(path_splitv);
					path_splitv = NULL;
				}

				/* Call that command */
				int exitVal = system(replace_path);
				if(exitVal != EXIT_SUCCESS)
				{
					message(1,s,stderr,"glyrc: cmd returned a value != EXIT_SUCCESS\n");
				}

				g_free(replace_path);
			}
			g_free(path);
		}
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

int main(int argc, char * argv[])
{
	int result = EXIT_SUCCESS;

	// make sure to init everything and destroy again
	Gly_init();
	atexit(Gly_cleanup);

	GlyFetcherInfo * info = Gly_get_plugin_info();
	GlyFetcherInfo * elem = info;

	Gly_free_plugin_info(&info);
	if(info == NULL) puts("Oh dear.");


	if(argc >= 3)
	{
		// The struct that control this beast
		GlyQuery my_query;

		// set it on default values
		Gly_init_query(&my_query);

		// Good enough for glyrc
		GlyOpt_verbosity(&my_query,2);


		my_query.type = GET_COVERART;

		gchar * write_to = NULL;
		parse_commandline_general(argc-1, argv+1, &my_query,&write_to);

		if(write_to == NULL)
		{
			g_printerr("- Not path specified. Assuming current working directory.\n");
			write_to = ".";
		}

		g_printerr("Will write to %s\n",write_to);

		// Set the callback - it will do all the actual work
		int item_counter = 0;
		GlyOpt_dlcallback(&my_query, callback, &item_counter);

		if(my_query.type != GET_UNSURE)
		{
			// Now start searching!
			int length = -1;
			enum GLYR_ERROR get_error = GLYRE_OK;
			GlyMemCache * my_list = Gly_get(&my_query, &get_error, &length);

			if(my_list)
			{
				if(get_error == GLYRE_OK)
				{
					/* This is the place where you would work with the cachelist *
					   As the callback is used in glyrc this is just plain empty *
					   Useful if you need to cache the data (e.g. for batch jobs *
					   Left only for the reader's informatiom, no functions here *
					 */

					message(2,&my_query,stderr,"- In total %d items found.\n",length);
				}

				// Free all downloaded buffers
				Gly_free_list(my_list);

			}
			else if(get_error != GLYRE_OK)
			{
				message(1,&my_query,stderr,"E: %s\n",Gly_strerror(get_error));
			}


			// Clean memory alloc'd by settings
			Gly_destroy_query( &my_query);
		}
		/* Translator mode - simple interface to google translator */
	}
	else if(argc >= 2 && !strcmp(argv[1],"-V"))
	{
		print_version(NULL);
	}
	else 
	{
		help_short(NULL);
	}

	// byebye
	return result;
}

//* --------------------------------------------------------- */
// ------------------End of program-------------------------- //
/* ---------------------------------------------------------- */
