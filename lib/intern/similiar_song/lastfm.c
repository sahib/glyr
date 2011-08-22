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

const gchar * similiar_song_lastfm_url(GlyrQuery * sets)
{
    return  "http://ws.audioscrobbler.com/2.0/?method=track.getsimilar&artist=%artist%&track=%title%&api_key="API_KEY_LASTFM;
}

#define TRACK_BEGIN "<track>"
#define TRACK_ENDIN "</track>"

#define NAME_BEGIN "<name>"
#define NAME_ENDIN "</name>"

#define ARTIST_BEGIN "<artist>"
#define ARTIST_ENDIN "</artist>"

#define MATCH_BEGIN "<match>"
#define MATCH_ENDIN "</match>"

#define URL_BEGIN "<url>"
#define URL_ENDIN "</url>"

static gchar * in_tag(const gchar * string, const gchar * begin, const gchar * endin)
{
    gchar * bp = strstr(string,begin);
    if(bp != NULL)
    {
        gchar * ep = strstr(bp,endin);
        if(ep != NULL && ep > bp)
        {
            return copy_value(bp+strlen(begin),ep);
        }
    }
    return NULL;
}

GList * similiar_song_lastfm_parse(cb_object * capo)
{
    GList * r_list = NULL;

    gint urlc = 0;
    gchar * begin = capo->cache->data;
    gchar * endin = NULL;

    while((begin = strstr(begin, TRACK_BEGIN)) != NULL &&
            (endin = strstr(begin, TRACK_ENDIN)) != NULL &&
            continue_search(urlc,capo->s))
    {
        endin += strlen(TRACK_ENDIN);
        *(endin-1) = '\0';

        gchar * track   = in_tag(begin,NAME_BEGIN,NAME_ENDIN);
        gchar * match   = in_tag(begin,MATCH_BEGIN,MATCH_ENDIN);
        gchar * url     = in_tag(begin,URL_BEGIN,URL_ENDIN);

        gchar * artist = NULL;
        gchar * begin_artist = begin;
        gchar * endin_artist = NULL;
        if((begin_artist = strstr(begin_artist, ARTIST_BEGIN)) != NULL &&
           (endin_artist = strstr(begin_artist, ARTIST_ENDIN)) != NULL)
        {
            endin_artist += strlen(ARTIST_ENDIN);
            *(endin_artist-1) = '\0';

            artist  = in_tag(begin_artist,NAME_BEGIN,NAME_ENDIN);
        }

        gchar * composed = g_strdup_printf("%s\n%s\n%s\n%s\n",track,artist,match,url);
        if(composed != NULL)
        {
            GlyrMemCache * result = DL_init();

            result->data = composed;
            result->size = strlen(composed);
            result->dsrc = strdup(capo->url);

            r_list = g_list_prepend(r_list, result);
            urlc++;
        }

        if(track)
            g_free(track);
        if(artist)
            g_free(artist);
        if(match)
            g_free(match);
        if(url)
            g_free(url);

        begin=endin;
    }
    return r_list;
}

/*--------------------------------------------------------*/

MetaDataSource similar_song_lastfm_src =
{
    .name = "lastfm",
    .key  = 'l',
    .parser    = similiar_song_lastfm_parse,
    .get_url   = similiar_song_lastfm_url,
    .quality   = 90,
    .speed     = 90,
    .endmarker = NULL,
    .free_url  = false,
    .type      = GET_SIMILIAR_SONGS
};
