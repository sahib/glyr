/***********************************************************
 * This file is part of glyr
 * + a commnadline tool and library to download various sort of musicrelated metadata.
 * + Copyright (C) [2011]  [serztle]
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

static const gchar * similar_song_lastfm_url (GlyrQuery * sets)
{
    return  "http://ws.audioscrobbler.com/2.0/?method=track.getsimilar&artist=${artist}&track=${title}&api_key="API_KEY_LASTFM;
}

#define TRACK_BEGIN "<track>"

#define NAME_BEGIN "<name>"
#define NAME_ENDIN "</name>"

#define ARTIST_BEGIN "<artist>"

#define MATCH_BEGIN "<match>"
#define MATCH_ENDIN "</match>"

#define URL_BEGIN "<url>"
#define URL_ENDIN "</url>"

static GList * similar_song_lastfm_parse (cb_object * capo)
{
    GList * results = NULL;
    gchar * begin = capo->cache->data;

    while (continue_search (g_list_length (results),capo->s) && (begin = strstr (begin, TRACK_BEGIN) ) != NULL)
    {
        gchar * track  = get_search_value (begin,NAME_BEGIN,NAME_ENDIN);
        gchar * match  = get_search_value (begin,MATCH_BEGIN,MATCH_ENDIN);
        gchar * url    = get_search_value (begin,URL_BEGIN,URL_ENDIN);
        gchar * artist = get_search_value (strstr (begin,ARTIST_BEGIN),NAME_BEGIN,NAME_ENDIN);

        if (artist && track)
        {
            GlyrMemCache * result = DL_init();
            result->data = g_strdup_printf ("%s\n%s\n%s\n%s\n",track,artist,match,url);
            result->size = strlen (result->data);
            results = g_list_prepend (results, result);
        }

        g_free (track);
        g_free (artist);
        g_free (match);
        g_free (url);

        begin += sizeof (TRACK_BEGIN) - 1;
    }
    return results;
}

/////////////////////////////////

MetaDataSource similar_song_lastfm_src =
{
    .name = "lastfm",
    .key  = 'l',
    .parser    = similar_song_lastfm_parse,
    .get_url   = similar_song_lastfm_url,
    .quality   = 90,
    .speed     = 90,
    .endmarker = NULL,
    .free_url  = false,
    .type      = GLYR_GET_SIMILAR_SONGS
};
