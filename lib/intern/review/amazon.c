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
#include "../common/amazon.h"

static const gchar *review_amazon_url(GlyrQuery *settings)
{
    return generic_amazon_url(settings, "EditorialReview");
}

#define TheContent "<Content>"
#define TheEndofCt "</Content>"
static GList *review_amazon_parse(cb_object *capo)
{
    gchar *node = capo->cache->data;
    gsize conlen = (sizeof TheContent) - 1;
    GList *result_list = NULL;
    while(continue_search(g_list_length(result_list), capo->s) && (node = strstr(node + conlen, TheContent)) != NULL) {
        gchar *endOfText = strstr(node + conlen, TheEndofCt);
        gchar *text = copy_value(node + conlen, endOfText);
        if(text != NULL) {
            /* Ignore reviews with 350 chars
             * as mostly just advertisement */
            if((endOfText - (node + conlen)) > 350) {
                GlyrMemCache *result = DL_init();
                result->data = text;
                result->size = result->data ? strlen(result->data) : 0;
                result_list = g_list_prepend(result_list, result);
            }
        }
    }
    return result_list;
}

/////////////////////////////////

MetaDataSource review_amazon_src = {
    .name = "amazon",
    .key  = 'a',
    .parser    = review_amazon_parse,
    .get_url   = review_amazon_url,
    .type      = GLYR_GET_ALBUM_REVIEW,
    .quality   = 25,
    .speed     = 70,
    .endmarker = NULL,
    .free_url  = true,
    .lang_aware = true
};
