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

#define INFO_BEGIN "</div><div style=\"text-align:center;\">"
#define INFO_ENDIN " <a href=\""

static const char *lyrics_lyricsreg_url(GlyrQuery *s)
{
    return "http://www.lyricsreg.com/lyrics/${artist}/${title}/";
}

static GList *lyrics_lyricsreg_parse(cb_object *capo)
{
    GList *result_list = NULL;
    gchar *start = strstr(capo->cache->data, INFO_BEGIN);

    if(start != NULL) {
        start += (sizeof INFO_BEGIN) - 1;
        gchar *end = strstr(start, INFO_ENDIN);
        if(end != NULL) {
            * (end) = 0;
            gchar *no_br_tags = strreplace(start, "<br />", NULL);
            if(no_br_tags != NULL) {
                GlyrMemCache *tmp = DL_init();
                tmp->data = beautify_string(no_br_tags);
                tmp->size = tmp->data ? strlen(tmp->data) : 0;
                g_free(no_br_tags);

                if(tmp->data != NULL) {
                    result_list = g_list_prepend(result_list, tmp);
                }
            }
        }
    }
    return result_list;
}

/////////////////////////////////

MetaDataSource lyrics_lyricsreg_src = {
    .name = "lyricsreg",
    .key  = 'r',
    .parser    = lyrics_lyricsreg_parse,
    .get_url   = lyrics_lyricsreg_url,
    .type      = GLYR_GET_LYRICS,
    .quality   = 42,
    .speed     = 90,
    .endmarker = NULL,
    .free_url  = false
};
