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
#include "../common/musicbrainz.h"

/* Possible cases:
------------------
artist -> artist
album  -> album
title  -> title
---------------
artist && album && title -> title
artist && title          -> title
artist && album          -> album
*/

/////////////////////////////////

/* Wrap around the (a bit more) generic versions */
static GList *tags_musicbrainz_parse(cb_object *capo)
{
    GList *results = NULL;
    gint mbid_marker = 0;
    while(continue_search(g_list_length(results), capo->s)) {
        GlyrMemCache   *info = generic_musicbrainz_parse(capo, &mbid_marker, "tags");
        if(info == NULL) {
            break;
        }

        gint type_num = please_what_type(capo->s);
        gchar *tag_node = info->data;
        while((tag_node = strstr(tag_node + 1, "<tag"))) {
            gchar *tag_begin = strchr(tag_node + 1, '>');
            if(!tag_begin) {
                continue;
            }

            tag_begin++;
            gchar *tag_endin = strchr(tag_begin, '<');
            if(!tag_endin) {
                continue;
            }

            gchar *value = copy_value(tag_begin, tag_endin);
            if(value != NULL) {
                if(strlen(value) > 0) {
                    GlyrMemCache *tmp = DL_init();
                    tmp->data = value;
                    tmp->size = tag_endin - tag_begin;
                    tmp->type = type_num;
                    tmp->dsrc = g_strdup(info->dsrc);

                    results = g_list_prepend(results, tmp);
                }
            }
        }
        DL_free(info);
    }
    return results;
}

/////////////////////////////////

static const gchar *tags_musicbrainz_url(GlyrQuery *sets)
{
    return generic_musicbrainz_url(sets);
}

/////////////////////////////////

MetaDataSource tags_musicbrainz_src = {
    .name = "musicbrainz",
    .key  = 'm',
    .parser    = tags_musicbrainz_parse,
    .get_url   = tags_musicbrainz_url,
    .endmarker = NULL,
    .free_url  = true,
    .quality   = 90,
    .speed     = 90,
    .type      = GLYR_GET_TAGS
};
