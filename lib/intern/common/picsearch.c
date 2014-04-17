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
#include "picsearch.h"

const gchar *generic_picsearch_url(GlyrQuery *s, const char *fmt)
{
    /* picsearch offers a nice way to set img_min / img_max */
    gchar *base_url = "http://www.picsearch.com/index.cgi?q=%s&face=both&orientation=square&size=%dt%d";

    gint img_min_size = s->img_min_size;
    if(img_min_size == -1) {
        img_min_size = 0;
    }

    gint img_max_size = s->img_max_size;
    if(img_max_size == -1) {
        img_max_size = INT_MAX;
    }

    return g_strdup_printf(base_url, fmt, img_min_size, img_max_size);
}

/////////////////////////////////

#define IMG_HOOK "div class=\"thumbnailTop\""
#define IMG_HOOK_BEGIN "<a rel=\"nofollow\" href=\""
#define IMG_HOOK_ENDIN "\"><img src=\""

static GlyrMemCache *parse_details_page(GlyrMemCache *to_parse)
{
    GlyrMemCache *result = NULL;
    if(to_parse != NULL) {
        char *start = strstr(to_parse->data, IMG_HOOK);
        if(start != NULL) {
            char *img_url = get_search_value(start, IMG_HOOK_BEGIN, IMG_HOOK_ENDIN);
            if(img_url != NULL) {
                result = DL_init();
                result->data = img_url;
                result->size = strlen(img_url);
                result->dsrc = g_strdup(to_parse->dsrc);
            }
        }
    }
    return result;
}

/////////////////////////////////

#define NODE "<div class=\"imgContainer\">"
#define NODE_NEEDS_TO_BEGIN "/imageDetail.cgi"

GList *generic_picsearch_parse(cb_object *capo)
{
    GList *result_list = NULL;

    gchar *node = capo->cache->data;
    gint nodelen = (sizeof NODE) - 1;

    node = strstr(node, "<div id=\"results_table\">");

    int items = 0, tries = 0;
    const int MAX_TRIES = capo->s->number * 4;

    while(continue_search(items, capo->s)         &&
            (node = strstr(node, "<a href=\""))  &&
            tries++ < MAX_TRIES) {
        node += nodelen;
        gchar *details_url = get_search_value(node, "<a href=\"", "\" ");
        if(details_url != NULL && strncmp(details_url, NODE_NEEDS_TO_BEGIN, sizeof(NODE_NEEDS_TO_BEGIN) - 1) == 0) {
            gchar *full_url = g_strdup_printf("www.picsearch.com%s", details_url);
            if(full_url != NULL) {
                GlyrMemCache *to_parse = download_single(full_url, capo->s, NULL);
                if(to_parse != NULL) {
                    GlyrMemCache *result = parse_details_page(to_parse);
                    if(result != NULL) {
                        result_list = g_list_prepend(result_list, result);
                        items++;
                    }
                    DL_free(to_parse);
                }
                g_free(full_url);
            }
            g_free(details_url);
        }
    }
    return result_list;
}
