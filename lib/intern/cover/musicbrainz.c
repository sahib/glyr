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

#include "../../stringlib.h"
#include "../../core.h"

/////////////////////////////////

static const char * cover_musicbrainz_url (GlyrQuery * q)
{
    return "http://musicbrainz.org/ws/2/release?query=${album}&limit=10&offset=0";
}

/////////////////////////////////

#define COVERART "<div class=\"cover-art\">"
#define AMZ_URL_START "\"http://ecx.images-amazon.com/"

/*
 * This is silly overall,
 * but coverartarchive.org does not seem to work fully yet.
 */
static GlyrMemCache * parse_web_page (GlyrMemCache * page)
{
    GlyrMemCache * retv = NULL;
    if (page && page->data)
    {
        char * begin = strstr (page->data,COVERART);
        if (begin != NULL)
        {
            char * amz_url = strstr (begin,AMZ_URL_START);
            if (amz_url != NULL)
            {
                char * img_url = get_search_value (amz_url,"\"","\"");
                if (img_url != NULL)
                {
                    retv = DL_init();
                    retv->dsrc = g_strdup (page->dsrc);
                    retv->data  = img_url;
                    retv->size  = strlen (img_url);
                }
            }
        }
        DL_free (page);
    }
    return retv;
}

/////////////////////////////////

#define NODE "<release ext:score="
#define DL_URL "http://musicbrainz.org/release/%s"

/////////////////////////////////

static GList * cover_musicbrainz_parse (cb_object * capo)
{
    GList * result_list = NULL;

    char * node = capo->cache->data;
    while (continue_search (g_list_length (result_list),capo->s) && (node = strstr (node + 1,NODE) ) )
    {
        char * album  = get_search_value (node,"<title>","</title>");
        char * artist = get_search_value (node,"<name>" ,"</name>" );

        if (levenshtein_strnormcmp (capo->s,artist,capo->s->artist) <= capo->s->fuzzyness &&
                levenshtein_strnormcmp (capo->s,album ,capo->s->album ) <= capo->s->fuzzyness)
        {
            char * ID = get_search_value (node,"id=\"","\">");
            if (ID != NULL)
            {
                char * url = g_strdup_printf (DL_URL,ID);
                if (url != NULL)
                {
                    GlyrMemCache * item = parse_web_page (download_single (url,capo->s,NULL) );
                    if (item != NULL)
                    {
                        result_list = g_list_prepend (result_list,item);
                    }
                }
                g_free (url);
            }
            g_free (ID);
        }
        g_free (artist);
        g_free (album);
    }

    return result_list;
}

/////////////////////////////////

MetaDataSource cover_musicbrainz_src =
{
    .name      = "musicbrainz",
    .key       = 'z',
    .parser    = cover_musicbrainz_parse,
    .get_url   = cover_musicbrainz_url,
    .type      = GLYR_GET_COVERART,
    .quality   = 85,
    .speed     = 70,
    .endmarker = NULL,
    .free_url  = false
};
