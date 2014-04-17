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

static const gchar *cover_albumart_url(GlyrQuery *sets)
{
    gint i = sets->img_min_size;
    gint e = sets->img_max_size;

    if((e >= 50 || e == -1) && (i == -1 || i < 450)) {
        return "http://www.albumart.org/index.php?searchkey=${artist}&itempage=1&newsearch=1&searchindex=Music";
    }
    return NULL;
}

/////////////////////////////////

#define NODE_START "<div id=\"main\">"
#define NODE_NEXT "<li><div style=\""
#define AMZ "http://ecx.images-amazon.com/images/"
#define IMG_FORMAT ".jpg"

static GList *cover_albumart_parse(cb_object *capo)
{
    GList *result_list = NULL;
    gchar *node = strstr(capo->cache->data, NODE_START);
    if(node != NULL) {
        /* Decide what size we want */
        gsize size_it = 2;
        if(capo->s->img_max_size < 450 && capo->s->img_max_size != -1 && capo->s->img_min_size < 160) {
            size_it = 1;
        }

        /* Go through all nodes */
        while(continue_search(g_list_length(result_list), capo->s) && (node = strstr(node + (sizeof NODE_NEXT) - 1, NODE_NEXT))) {
            gchar *img_tag = node;
            gchar *img_end = NULL;

            gchar *album_name = get_search_value(node, "title=\"", "\"");
            if(levenshtein_strnormcmp(capo->s, album_name, capo->s->album) <= capo->s->fuzzyness) {
                for(gsize it = 0; it < size_it; it++, img_tag += (sizeof AMZ) - 1) {
                    if((img_tag = strstr(img_tag, AMZ)) == NULL) {
                        break;
                    }
                }

                if((img_end  = strstr(img_tag, IMG_FORMAT)) != NULL) {
                    gchar *img_url = copy_value(img_tag, img_end);
                    if(img_url != NULL) {
                        GlyrMemCache *result = DL_init();
                        result->data = g_strdup_printf(AMZ"%s"IMG_FORMAT, img_url);
                        result->size = strlen(result->data);
                        result_list = g_list_prepend(result_list, result);
                        g_free(img_url);
                    }
                }
            }
            g_free(album_name);
        }
    }
    return result_list;
}

/////////////////////////////////

MetaDataSource cover_albumart_src = {
    .name      = "albumart",
    .key       = 'b',
    .parser    = cover_albumart_parse,
    .get_url   = cover_albumart_url,
    .type      = GLYR_GET_COVERART,
    .quality   = 80,
    .speed     = 65,
    .free_url  = false
};
