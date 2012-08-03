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
#include "../common/mbid_lookup.h"


//////////////////////////////////////////////////

static const char * cover_coverartarchive_url (GlyrQuery * qry)
{
    return "http://musicbrainz.org/ws/2/release?query=artist:${artist}%20AND%20release:${album}";
}

//////////////////////////////////////////////////

#define IMAGE_NODE "\"image\":\""

/* This should work, but apparently there is no real data yet there... */
static GList * parse_archive_json (GlyrMemCache * input, GlyrQuery * qry)
{
    GList * result_list = NULL;
    char * node = input->data;

    puts (input->data);
    while ( (node = strstr (node + sizeof (IMAGE_NODE), IMAGE_NODE) ) )
    {
        char * url = copy_value (node, strstr (node + sizeof (IMAGE_NODE), "\"") );
        if (url != NULL)
        {
            GlyrMemCache * item = DL_init();
            item->data = url;
            item->size = strlen (url);
            item->dsrc = g_strdup (input->dsrc);

            result_list = g_list_prepend (result_list, item);
        }
    }
    return result_list;
}

//////////////////////////////////////////////////

#define API_ROOT "http://coverartarchive.org/release/%s/"


//////////////////////////////////////////////////

static GList * cover_coverartarchive_parse (cb_object * capo)
{
    GList *result_list = NULL;
    char * mbid = mbid_parse_data (capo->cache, "release", "title", capo->s->album, capo->s);
    if (mbid != NULL)
    {
        char * full_url = g_strdup_printf (API_ROOT, mbid);
        if (full_url != NULL)
        {
            puts (full_url);
            GlyrMemCache * json_data = download_single (full_url, capo->s, NULL);
            if (json_data != NULL)
            {
                result_list = parse_archive_json (json_data, capo->s);
                DL_free (json_data);
            }
            g_free (full_url);
        }
    }
    return result_list;
}

//////////////////////////////////////////////////

MetaDataSource cover_coverartarchive_src =
{
    .name      = "coverartarchive",
    .key       = 'z',
    .parser    = cover_coverartarchive_parse,
    .get_url   = cover_coverartarchive_url,
    .type      = GLYR_GET_COVERART,
    .quality   = 90,
    .speed     = 80,
    .endmarker = NULL,
    .free_url  = false
};
