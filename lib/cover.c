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
#include <stdbool.h>
#include <unistd.h>

#include "core.h"
#include "types.h"
#include "stringop.h"

//Include plugins:
#include "cover/last_fm.h"
#include "cover/coverhunt.h"
#include "cover/discogs.h"
#include "cover/amazon.h"
#include "cover/allmusic_com.h"
#include "cover/lyricswiki.h"
#include "cover/albumart.h"
#include "cover/google.h"

#define GOOGLE_COLOR C_B"g"C_R"o"C_Y"o"C_B"g"C_G"l"C_R"e"

#define ALBE "<div id=\"pagination\""
#define COLE "<div id=\"footer\">"

// Add yours here.
GlyPlugin cover_providers[] =
{
    // Start of safe group
    {"last.fm",    "l", C_"last"C_R"."C_"fm", false, {cover_lastfm_parse,      cover_lastfm_url,     NULL, false}, GRP_SAFE | GRP_FAST},
    {"amazon",     "a", C_Y"amazon",          false, {cover_amazon_parse,      cover_amazon_url,     NULL, false}, GRP_SAFE | GRP_FAST},
    {"lyricswiki", "w", C_C"lyricswiki",      false, {cover_lyricswiki_parse,  cover_lyricswiki_url, NULL, false}, GRP_SAFE | GRP_FAST},
    {"google",     "g", GOOGLE_COLOR,         false, {cover_google_parse,      cover_google_url,     NULL, true }, GRP_USFE | GRP_FAST},
    {"albumart",   "b", C_R"albumart",        false, {cover_albumart_parse,    cover_albumart_url,   ALBE, false}, GRP_USFE | GRP_FAST},
    {"discogs",    "d", C_"disc"C_Y"o"C_"gs", false, {cover_discogs_parse,     cover_discogs_url,    NULL, false}, GRP_USFE | GRP_SLOW},
    {"allmusic",   "m", C_"all"C_C"music",    false, {cover_allmusic_parse,    cover_allmusic_url,   NULL, false}, GRP_SPCL | GRP_SLOW},
    {"coverhunt",  "c", C_G"coverhunt",       false, {cover_coverhunt_parse,   cover_coverhunt_url,  COLE, false}, GRP_SPCL | GRP_FAST},
    { NULL,        NULL, NULL,                false, {NULL,                    NULL,                 NULL, false}, GRP_NONE | GRP_NONE},
};

bool size_is_okay(int sZ, int min, int max)
{
    if((min == -1 && max == -1) ||
       (min == -1 && max >= sZ) ||
       (min <= sZ && max == -1) ||
       (min <= sZ && max >= sZ)  )
       return true;

    return false;
}

GlyPlugin * glyr_get_cover_providers(void)
{
    return copy_table(cover_providers,sizeof(cover_providers));
}

static GlyCacheList * cover_callback(cb_object * capo)
{
    // in this 'pseudo' callback we copy
    // the downloaded cache, and add the source url
    GlyCacheList * ls = NULL;
    GlyMemCache  * dl = DL_copy(capo->cache);

    if(dl != NULL)
    {
	int usersig = GLYRE_OK;

        dl->dsrc = strdup(capo->url);
	if(dl->type == TYPE_NOIDEA)
	    dl->type = TYPE_COVER;

        // call user defined callback if any
        if(capo->s->callback.download)
	{
            usersig = capo->s->callback.download(dl,capo->s);
	}

	if(usersig == GLYRE_OK)
	{
		ls = DL_new_lst();
		ls->usersig = usersig; 
        	DL_add_to_list(ls,dl);
	}
	else
	{
		DL_free(dl);
	}
    }
    return ls;
}

// Place any bad URLs that should never ever get out
// Prefer to handle this in the plugin though!
const char * URLblacklist[] =
{
    "http://ecx.images-amazon.com/images/I/11J2DMYABHL.jpg", // blank image.
    NULL
};

static GlyCacheList * cover_finalize(GlyCacheList * result, GlyQuery * settings)
{
    // Only NULL when finalizing()
    if(result == NULL)
    {
	if(settings->itemctr)
	{
		glyr_message(2,settings,stderr,C_R"* "C_"Got in total %d images!\n",settings->itemctr);
	}
	return NULL;
    }

    GlyCacheList * dl_list = NULL;
    if(result)
    {
        if(settings->download)
        {
            cb_object  * urlplug_list = calloc(result->size+1,sizeof(cb_object));
            if(urlplug_list)
            {
                /* Ignore double URLs */
                flag_double_urls(result,settings);

                /* Watch out for blacklisted URLs */
                flag_blacklisted_urls(result,URLblacklist,settings);

		flag_invalid_format(result,settings);

                size_t i = 0;
                int ctr = 0;
                for(i = 0; i < result->size; i++)
                {
                    if(!result->list[i]->error)
                    {
                        plugin_init(&urlplug_list[ctr], result->list[i]->data, cover_callback, settings, NULL, NULL, true);
                        ctr++;
                    }
                }

                dl_list = invoke(urlplug_list,ctr,settings->parallel,settings->timeout * ctr, settings);
		if(dl_list != NULL)
		{
                	glyr_message(2,settings,stderr,C_G"* "C_"Succesfully downloaded %d image.\n",dl_list->size);
		}
                free(urlplug_list);
            }
        }
        else
        {
            /* Ignore double URLs */
            flag_double_urls(result,settings);

            /* Watch out for blacklisted URLs */
            flag_blacklisted_urls(result,URLblacklist,settings);
		
  	    flag_invalid_format(result,settings);

            size_t i = 0;
            for( i = 0; i < result->size; i++)
            {
                if(result->list[i] && result->list[i]->error == ALL_OK)
                {
                    if(!dl_list) dl_list = DL_new_lst();
                    GlyMemCache * r_copy = DL_copy(result->list[i]);
                    DL_add_to_list(dl_list,r_copy);
                }
            }
        }
    }
    return dl_list;
}

GlyCacheList * get_cover(GlyQuery * settings)
{
    GlyCacheList * res = NULL;
    if (settings && settings->artist && settings->album)
    {
        // validate size
        if(settings->cover.min_size <= 0)
            settings->cover.min_size = -1;

        if(settings->cover.max_size <= 0)
            settings->cover.max_size = -1;

        res = register_and_execute(settings, cover_finalize);
    }
    else
    {
        glyr_message(1,settings,stderr,C_R"* "C_"%s is needed to download covers.\n",settings->artist ? "Album" : "Artist");
    }
    return res;
}
