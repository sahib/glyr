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

//////////////////////////////////

char *mbid_parse_data(GlyrMemCache *data, const char *lookup_entity, const char *find_entity, const char *compre_entity, GlyrQuery *qry)
{
    char *key = g_strdup_printf("<%s ", lookup_entity);
    size_t keylen = strlen(key);
    char *node = data->data;
    char *result = NULL;

    char *find_ent_start = g_strdup_printf("<%s>", find_entity);
    char *find_ent_end   = g_strdup_printf("</%s>", find_entity);

    while((node = strstr(node + keylen, key))) {
        char *name = get_search_value(node, find_ent_start, find_ent_end);
        if(name && levenshtein_strnormcmp(qry, name, compre_entity) <= qry->fuzzyness) {
            result = get_search_value(node, "id=\"", "\"");
            g_free(name);
            break;
        }
        g_free(name);
    }

    g_free(find_ent_start);
    g_free(find_ent_end);
    g_free(key);
    return result;
}


//////////////////////////////////

#define LOOKUP_QUERY "http://musicbrainz.org/ws/2/%s?query=%s:%s"

//////////////////////////////////

char *mbid_lookup(const char *query, GLYR_DATA_TYPE type, GlyrQuery *qry)
{
    char *result_mbid = NULL;
    if(query == NULL) {
        return result_mbid;
    }

    const char *lookup_entity = "";
    const char *compre_entity = qry->artist;
    const char *find_entity = "name";

    switch(type) {
    case GLYR_TYPE_TAG_ARTIST:
        lookup_entity = "artist";
        compre_entity = qry->artist;
        break;
    case GLYR_TYPE_TAG_ALBUM:
        lookup_entity = "release";
        compre_entity = qry->album;
        find_entity = "title";
        break;
    case GLYR_TYPE_TAG_TITLE:
        lookup_entity = "work";
        compre_entity = qry->title;
        break;
    default:
        lookup_entity = "artist";
        compre_entity = qry->artist;
        find_entity = "name";
        break;
    }

    char *lookup_url = g_strdup_printf(LOOKUP_QUERY, lookup_entity, lookup_entity, query);
    GlyrMemCache *parseable_data = download_single(lookup_url, qry, NULL);
    if(parseable_data != NULL) {
        result_mbid = mbid_parse_data(parseable_data, lookup_entity, find_entity, compre_entity, qry);
        DL_free(parseable_data);
    }

    g_free(lookup_url);
    return result_mbid;
}

//////////////////////////////////
