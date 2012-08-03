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

static const gchar * albumlist_musicbrainz_url (GlyrQuery * sets)
{
    return "http://musicbrainz.org/ws/1/release/?type=xml&artist=${artist}&releasetypes=\"Official\"";
}

/////////////////////////////////////////////////////////////

#define ALBUM_BEGIN "<release type=\"Album Official\""
#define ALBUM_ENDIN "\" id=\""

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

static GList * albumlist_musicbrainz_parse (cb_object * capo)
{
    GList * result_list = NULL;
    gchar * node = capo->cache->data;

    while (continue_search (g_list_length (result_list),capo->s) && (node = strstr (node+1,ALBUM_BEGIN) ) != NULL)
    {
        gchar * name = get_search_value (node,TITLE_BEGIN,TITLE_ENDIN);
        if (name != NULL && is_in_list (result_list,name) == false)
        {
            GlyrMemCache * result = DL_init();
            result->data = name;
            result->size = (result->data) ? strlen (result->data) : 0;
            result_list = g_list_prepend (result_list,result);
        }
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
