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

#define API_URL "http://www.slothradio.com/covers/?adv=1&artist=%s&album=%s&imgsize=%c&locale=%s&sort=salesrank"

#define RESULT_LIST_START "<!-- RESULT LIST START -->"
#define RESULT_ITEM_START "<!-- RESULT ITEM START -->"

#define RESULT_LIST_END "<!-- RESULT LIST END -->"
#define RESULT_ITEM_END "<!-- RESULT ITEM END -->"

///////////////////////

#define STREQ(s1,s2) (g_ascii_strcasecmp(s1,s2) == 0)

static const char *cover_slothradio_url(GlyrQuery *s)
{
    const char *locale = "us";
    if(STREQ(s->lang, "uk") || STREQ(s->lang, "de")) {
        locale = s->lang;
    }

    /*
     * Possible image sizes:
     * x = largest available
     * l = large  (300**2)
     * m = medium (130**2)
     * s = small  (50 **2)
     **/
    char image_size = 'x';
    if(s->img_max_size != -1) {
        if(s->img_max_size < 75) {
            image_size = 's';
        } else if(s->img_max_size < 150) {
            image_size = 'm';
        } else if(s->img_max_size < 350) {
            image_size = 'l';
        }
    }

    return g_strdup_printf(API_URL, s->artist, s->album, image_size, locale);
}

///////////////////////

static bool check_size(GlyrQuery *q, char *node)
{
    bool rc = false;
    char *width  = get_search_value(node, "width=\"", "\"");
    char *height = get_search_value(node, "height=\"", "\"");

    if(width && height) {
        int w = strtol(width, NULL, 10);
        int h = strtol(height, NULL, 10);

        if(size_is_okay(w, q->img_min_size, q->img_max_size) &&
                size_is_okay(h, q->img_min_size, q->img_max_size)) {
            rc = true;
        }
    }
    g_free(width);
    g_free(height);

    return rc;
}

///////////////////////

static GList *cover_slothradio_parse(cb_object *capo)
{
    GList *result_list = NULL;

    const char *bound_start = strstr(capo->cache->data, RESULT_LIST_START);
    if(bound_start == NULL) {
        return NULL;
    }

    const char *bound_end  = strstr(bound_start, RESULT_LIST_END);
    if(bound_end == NULL) {
        return NULL;
    }

    char *node = (char *) bound_start;
    while((node = strstr(node + sizeof(RESULT_ITEM_START), RESULT_ITEM_START)) != NULL) {
        if(node >= bound_end) {
            break;
        }

        char *url = get_search_value(node, "img src=\"", "\"");
        if(url != NULL) {
            if(check_size(capo->s, node)) {
                GlyrMemCache *result = DL_init();
                result->dsrc = g_strdup(capo->url);
                result->data = url;
                result->size = strlen(url);

                result_list = g_list_prepend(result_list, result);
            } else {
                g_free(url);
            }
        }

        if(continue_search(g_list_length(result_list), capo->s) == false) {
            break;
        }
    }

    return result_list;
}

///////////////////////

MetaDataSource cover_slothradio_src = {
    .name      = "slothradio",
    .key       = 's',
    .parser    = cover_slothradio_parse,
    .get_url   = cover_slothradio_url,
    .type      = GLYR_GET_COVERART,
    .quality   = 80,
    .speed     = 80,
    .endmarker = NULL,
    .free_url  = true
};
