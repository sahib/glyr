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

#define API_KEY API_KEY_LASTFM

static const char * similar_lastfm_url (GlyrQuery * sets)
{
    return "http://ws.audioscrobbler.com/2.0/?method=artist.getsimilar&artist=${artist}&api_key="API_KEY;
}

#define NAME_BEGIN "<name>"
#define NAME_ENDIN "</name>"

#define MATCH_BEGIN "<match>"
#define MATCH_ENDIN "</match>"

#define URL_BEGIN "<url>"
#define URL_ENDIN "</url>"

#define IMAGE_S_BEGIN "<image size=\"small\">"
#define IMAGE_M_BEGIN "<image size=\"medium\">"
#define IMAGE_L_BEGIN "<image size=\"large\">"
#define IMAGE_E_BEGIN "<image size=\"extralarge\">"
#define IMAGE_X_BEGIN "<image size=\"mega\">"
#define IMAGE_ENDIN   "</image>"

static GList * similar_lastfm_parse (cb_object * capo)
{
    GList * results = NULL;
    gchar * find = capo->cache->data;
    while (continue_search (g_list_length (results),capo->s) && (find = strstr (find+1, "<artist>") ) != NULL)
    {
        gchar * name  = get_search_value (find,NAME_BEGIN,NAME_ENDIN);
        gchar * match = get_search_value (find,MATCH_BEGIN,MATCH_ENDIN);
        gchar * url   = get_search_value (find,URL_BEGIN,URL_ENDIN);

        gchar * img_s = get_search_value (find,IMAGE_S_BEGIN,IMAGE_ENDIN);
        gchar * img_m = get_search_value (find,IMAGE_M_BEGIN,IMAGE_ENDIN);
        gchar * img_l = get_search_value (find,IMAGE_L_BEGIN,IMAGE_ENDIN);
        gchar * img_e = get_search_value (find,IMAGE_E_BEGIN,IMAGE_ENDIN);
        gchar * img_x = get_search_value (find,IMAGE_X_BEGIN,IMAGE_ENDIN);
        gchar * composed = g_strdup_printf ("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",name,match,url,img_s,img_m,img_l,img_e,img_x);

        if (composed != NULL)
        {
            GlyrMemCache * result = DL_init();
            result->data = composed;
            result->size = strlen (composed);
            results = g_list_prepend (results, result);
        }

        if (results != NULL)
        {
            results = g_list_reverse (results);
        }

        g_free (name);
        g_free (match);
        g_free (url);
        g_free (img_s);
        g_free (img_m);
        g_free (img_l);
        g_free (img_e);
        g_free (img_x);
    }
    return results;
}

/////////////////////////////////

MetaDataSource similar_artist_lastfm_src =
{
    .name = "lastfm",
    .key  = 'l',
    .parser    = similar_lastfm_parse,
    .get_url   = similar_lastfm_url,
    .quality   = 90,
    .speed     = 90,
    .endmarker = NULL,
    .free_url  = false,
    .type      = GLYR_GET_SIMILAR_ARTISTS
};
