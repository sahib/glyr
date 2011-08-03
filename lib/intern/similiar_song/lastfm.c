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

#define API_KEY API_KEY_LASTFM

const char * similiar_song_lastfm_url(GlyQuery * sets)
{
    return  "http://ws.audioscrobbler.com/2.0/?method=track.getsimilar&artist=%artist%&track=%title%&api_key="API_KEY;
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

#define IMAGE_S_BEGIN "<image size=\"small\">"
#define IMAGE_M_BEGIN "<image size=\"medium\">"
#define IMAGE_L_BEGIN "<image size=\"large\">"
#define IMAGE_E_BEGIN "<image size=\"extralarge\">"
#define IMAGE_X_BEGIN "<image size=\"mega\">"
#define IMAGE_ENDIN "</image>"

static char * in_tag(const char * string, const char * begin, const char * endin)
{
    char * bp = strstr(string,begin);
    if(bp != NULL)
    {
        char * ep = strstr(bp,endin);
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

    int urlc = 0;
    char * begin = capo->cache->data;
    char * endin = NULL;

    while((begin = strstr(begin, TRACK_BEGIN)) != NULL &&
            (endin = strstr(begin, TRACK_ENDIN)) != NULL &&
            continue_search(urlc,capo->s))
    {
        endin += strlen(TRACK_ENDIN);
        *(endin-1) = '\0';

        char * track   = in_tag(begin,NAME_BEGIN,NAME_ENDIN);
        char * match   = in_tag(begin,MATCH_BEGIN,MATCH_ENDIN);
        char * url     = in_tag(begin,URL_BEGIN,URL_ENDIN);
        char * img_s   = in_tag(begin,IMAGE_S_BEGIN,IMAGE_ENDIN);
        char * img_m   = in_tag(begin,IMAGE_M_BEGIN,IMAGE_ENDIN);
        char * img_l   = in_tag(begin,IMAGE_L_BEGIN,IMAGE_ENDIN);
        char * img_e   = in_tag(begin,IMAGE_E_BEGIN,IMAGE_ENDIN);
        char * img_x   = in_tag(begin,IMAGE_X_BEGIN,IMAGE_ENDIN);

        char * artist = NULL;
        char * begin_artist = begin;
        char * endin_artist = NULL;
        if ((begin_artist = strstr(begin_artist, ARTIST_BEGIN)) != NULL &&
                (endin_artist = strstr(begin_artist, ARTIST_ENDIN)) != NULL)
        {
            endin_artist += strlen(ARTIST_ENDIN);
            *(endin_artist-1) = '\0';

            artist  = in_tag(begin_artist,NAME_BEGIN,NAME_ENDIN);
        }

        char * composed = strdup_printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s",track,artist,match,url,img_s,img_m,img_l,img_e,img_x);

        if(composed != NULL)
        {
            GlyMemCache * result = DL_init();

            result->data = composed;
            result->size = strlen(composed);
            result->dsrc = strdup(capo->url);

            r_list = g_list_prepend(r_list, result);
            urlc++;
        }

        if(track)
            free(track);
        if(artist)
            free(artist);
        if(match)
            free(match);
        if(url)
            free(url);
        if(img_s)
            free(img_s);
        if(img_m)
            free(img_m);
        if(img_l)
            free(img_l);
        if(img_e)
            free(img_e);
        if(img_x)
            free(img_x);

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
