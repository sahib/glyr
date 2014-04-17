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

#define TRACK_BEGIN "<li class=\"trackname\"><a href=\""
#define TRACK_ENDIN "\">"
#define TRACK_DESCR "</a>"

#define LIPWALK_DOMAIN "http://www.lipwalklyrics.com"
#define LIPWALK_URL LIPWALK_DOMAIN"/component/lyrics/search/index.php?search=${artist}%20${title}"

#define START "</script><div class=\"clearboth\"></div>"
#define END "<h5 class=\"bottomViewing\">"

#define IS_ON_SEARCH_PAGE "<title>Search results for"

static const gchar *lyrics_lipwalk_url(GlyrQuery *settings)
{
    return LIPWALK_URL;
}

/////////////////////////////////

static GlyrMemCache *parse_lyrics_page(GlyrMemCache *cache)
{
    gchar *start = NULL;
    gchar *end = NULL;
    gchar *content = NULL;

    GlyrMemCache *result_cache = NULL;
    if(cache && (start = strstr(cache->data, START)) != NULL) {
        start += (sizeof START) - 1;
        if(start && (end = strstr(start + (sizeof START) , END)) != NULL) {
            if(ABS(end - start) > 35) {
                * (end) = 0;
                content = strreplace(start, "<br />", NULL);
                if(content != NULL) {
                    result_cache = DL_init();
                    result_cache->data = content;
                    result_cache->size = strlen(content);
                    result_cache->dsrc = g_strdup(cache->dsrc);
                }
            }
        }
    }
    return result_cache;
}

/////////////////////////////////

static gboolean validate_track_description(GlyrQuery *query, gchar *description)
{
    gboolean result = FALSE;
    if(description != NULL) {
        gchar **splitv = g_strsplit(description, " - ", 0);
        if(splitv != NULL) {
            if(splitv[0] && splitv[1] != NULL) {
                if(levenshtein_strnormcmp(query, query->artist, splitv[0]) <= query->fuzzyness
                        && levenshtein_strnormcmp(query, query->title, splitv[1]) <= query->fuzzyness) {
                    result = TRUE;
                }
            }
            g_strfreev(splitv);
        }
    }
    return result;
}

/////////////////////////////////

static GList *lyrics_lipwalk_parse(cb_object *capo)
{
    GList *result_list  = NULL;
    if(strstr(capo->cache->data, IS_ON_SEARCH_PAGE) == NULL) {
        GlyrMemCache *result_cache = parse_lyrics_page(capo->cache);
        result_list = g_list_prepend(result_list, result_cache);
    } else { /* Oops, we're on the search results page, things are complicated now */
        /* Happens with "In Flames" - "Trigger" e.g.                          */
        gchar *search_node = capo->cache->data;
        gsize track_len = (sizeof TRACK_BEGIN) - 1;
        while(continue_search(g_list_length(result_list), capo->s) && (search_node = strstr(search_node + track_len, TRACK_BEGIN))) {
            search_node += track_len;
            gchar *track_end = strstr(search_node, TRACK_ENDIN);
            if(track_end != NULL) {
                gchar *lyrics_url = copy_value(search_node, track_end);
                if(lyrics_url != NULL) {
                    track_end += (sizeof TRACK_ENDIN) - 1;
                    gchar *track_descr = copy_value(track_end, strstr(track_end, TRACK_DESCR));
                    if(track_descr != NULL && validate_track_description(capo->s, track_descr) == TRUE) {
                        gchar *full_url = g_strdup_printf("%s%s", LIPWALK_DOMAIN, lyrics_url);
                        GlyrMemCache *lyrics_page = download_single(full_url, capo->s, NULL);
                        if(lyrics_page != NULL) {
                            GlyrMemCache *result_cache = parse_lyrics_page(lyrics_page);
                            if(result_cache != NULL) {
                                result_list = g_list_prepend(result_list, result_cache);
                            }
                            DL_free(lyrics_page);
                        }
                        g_free(track_descr);
                        g_free(full_url);
                    }
                    g_free(lyrics_url);
                }
            }
        }
    }
    return result_list;
}

/////////////////////////////////

MetaDataSource lyrics_lipwalk_src = {
    .name = "lipwalk",
    .key  = 'z',
    .parser    = lyrics_lipwalk_parse,
    .get_url   = lyrics_lipwalk_url,
    .type      = GLYR_GET_LYRICS,
    .quality   = 80,
    .speed     = 60,
    .endmarker = NULL,
    .free_url  = false
};
