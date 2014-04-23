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

#include "../../stringlib.h"
#include "../../core.h"

static const gchar * albumlist_musicbrainz_url (GlyrQuery * sets)
{
    return "http://musicbrainz.org/ws/1/release/?type=xml&artist=${artist}&limit=100";
}

/////////////////////////////////////////////////////////////

//#define ALBUM_BEGIN "<release type=\"Album Official\""
#define ALBUM_BEGIN "<release "
#define TYPE_BEGIN "type=\""
#define TYPE_ENDIN "\" "


#define ARTIST_BEGIN "<name>"
#define ARTIST_ENDIN "</name>"

#define TITLE_BEGIN "<title>"
#define TITLE_ENDIN "</title>"

/////////////////////////////////////////////////////////////

static bool is_in_list (GList * list, const char * to_cmp)
{
    bool rc = false;
    for (GList * elem = list; elem; elem = elem->next)
    {
        GlyrMemCache * item = elem->data;
        if (item != NULL)
        {
            if (g_ascii_strcasecmp (item->data,to_cmp) == 0)
            {
                rc = true;
                break;
            }
        }
    }

    return rc;
}

/////////////////////////////////////////////////////////////

static bool type_is_valid (const char * type)
{
    bool result = FALSE;
    if (type == NULL)
        return result;

    static const char * valid_types[] = {
        "Single Official",
        "Album Official",
        NULL
    };

    int check_idx = 0;
    while(valid_types[check_idx]) {
        if (g_strcmp0(valid_types[check_idx], type) == 0) {
            result = TRUE;
            break;
        }
        check_idx++;
    }

    return result;
}

/////////////////////////////////////////////////////////////

static GList * albumlist_musicbrainz_parse (cb_object * capo)
{
    GList * result_list = NULL;
    gchar * node = capo->cache->data;

    while (continue_search (g_list_length (result_list),capo->s) &&
            (node = strstr (node + 1, ALBUM_BEGIN) ) != NULL)
    {
        gchar * type = get_search_value (node, TYPE_BEGIN, TYPE_ENDIN);
        if (type_is_valid (type)) 
        {
            gchar * artist = get_search_value (node,ARTIST_BEGIN, ARTIST_ENDIN);
            if (artist != NULL && 
                    levenshtein_strnormcmp (capo->s, capo->s->artist, artist) <= capo->s->fuzzyness) {


                gchar * name = get_search_value (node,TITLE_BEGIN,TITLE_ENDIN);

                if (name != NULL && is_in_list (result_list,name) == false)
                {
                    GlyrMemCache * result = DL_init();
                    result->data = name;
                    result->size = (result->data) ? strlen (result->data) : 0;
                    result_list = g_list_prepend (result_list,result);
                }
                else 
                {
                    g_free (name);
                }
            }
            g_free (artist);
        }

        g_free (type);
    }
    return result_list;
}

/////////////////////////////////////////////////////////////

MetaDataSource albumlist_musicbrainz_src =
{
    .name = "musicbrainz",
    .key = 'm',
    .free_url = false,
    .parser  = albumlist_musicbrainz_parse,
    .get_url = albumlist_musicbrainz_url,
    .type    = GLYR_GET_ALBUMLIST,
    .quality = 95,
    .speed   = 95,
    .endmarker = NULL
};
