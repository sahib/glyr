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

#include "../../core.h"
#include "../../stringlib.h"

#define API_ENTRY "http://api.discogs.com/database/search?type=artist&q=${artist}"

/////////////////////////////////

/*
 * # Example Snippet (for type = release)
 * { # Start of item
 *   "style": ["Grunge"],
 *   "thumb": "http://api.discogs.com/image/R-90-2845667-1303704896.jpeg",
 *   "title": "Nirvana - Nirvana",
 *   "country": "Russia",
 *   "format": ["CD"],
 *   "uri": "/Nirvana-Nirvana/release/2845667",
 *   "label": "\u0414\u043e\u043c\u0430\u0448\u043d\u044f\u044f \u041a\u043e\u043b\u043b\u0435\u043a\u0446\u0438\u044f",
 *   "catno": "none",
 *   "year": "2001",
 *   "genre": ["Rock"],
 *   "resource_url": "http://api.discogs.com/releases/2845667",
 *   "type": "release",
 *   "id": 2845667
 *  }, # End of item
 *  {
 *     ..more data..
 *  }
 */

/* Note: "thumb": null is ignored! */
#define TITLE_SUBNODE "\"title\": \""
#define THUMB_SUBDNOE "\"thumb\": \""
#define FOLLR_SUBNODE "\"uri\": \""
#define NODE THUMB_SUBDNOE
#define ENDOF_SUBNODE "\","


/////////////////////////////////////////////////////

static bool check_artist_album (GlyrQuery * q, const char * artist)
{
    return levenshtein_strnormcmp (q,q->artist, artist) <= q->fuzzyness;

}

/////////////////////////////////////////////////////

static GlyrMemCache * transform_url (cb_object * s, const char * url)
{
    GlyrMemCache * rc = NULL;
    size_t rc_size = strlen (url);
    char * rc_url  = g_strdup (url);

    if (rc_url != NULL)
    {
        char * slash = strrchr (rc_url,'/');
        if (slash != NULL)
        {
            char * sp = strchr (slash,'-');
            if (sp != NULL)
            {
                char * ep = strchr (sp + 1, '-');
                if(ep != NULL) {
                    size_t rest_len = rc_size - (ep - rc_url) + 1;
                    memmove (sp,ep,rest_len);

                    rc = DL_init();
                    rc->data = (char*) rc_url;
                    rc->size = strlen (url);
                    rc->dsrc = g_strdup (s->url);
                }
            }
        }
    }
    return rc;
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

static const char * photos_discogs_url (GlyrQuery * q)
{
    return API_ENTRY;
}

/////////////////////////////////////////////////////

static GList * photos_discogs_parse (cb_object * capo)
{
    GList * result_list = NULL;

    /* Jump to the very first node 'directly' */
    gchar * node = capo->cache->data;

    while (continue_search (g_list_length (result_list),capo->s)
            && (node = strstr (node + (sizeof NODE) - 1,NODE) ) != NULL)
    {

        char * artist_album = get_search_value (node,TITLE_SUBNODE,ENDOF_SUBNODE);
        if (artist_album && check_artist_album (capo->s,artist_album) )
        {

            char * thumb_url = get_search_value (node,THUMB_SUBDNOE,ENDOF_SUBNODE);
            if (thumb_url)
            {

                GlyrMemCache * p = transform_url (capo,thumb_url);
                if (p != NULL)
                {
                    result_list = g_list_prepend (result_list,p);
                }
                g_free (thumb_url);
            }
        }
        g_free (artist_album);
    }

    return result_list;
}

/////////////////////////////////

MetaDataSource photos_discogs_src =
{
    .name      = "discogs",
    .key       = 'd',
    .parser    = photos_discogs_parse,
    .get_url   = photos_discogs_url,
    .type      = GLYR_GET_ARTIST_PHOTOS,
    .quality   = 60,
    .speed     = 70,
    .endmarker = NULL,
    .free_url  = false
};
