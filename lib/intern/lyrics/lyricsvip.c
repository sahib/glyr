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

#define LV_URL "http://www.lyricsvip.com/%s/%s-Lyrics.html"

/////////////////////////////////

static const gchar *lyrics_lyricsvip_url(GlyrQuery *settings)
{
    gchar *result = NULL;
    gchar *artist_clean = strreplace(settings->artist, " ", "-");
    if(artist_clean != NULL) {
        gchar *title_clean =  strreplace(settings->title, " ", "-");
        if(title_clean != NULL) {
            gchar *prep_artist = curl_easy_escape(NULL, artist_clean, 0);
            gchar *prep_title  = curl_easy_escape(NULL, title_clean, 0);
            result = g_strdup_printf(LV_URL, prep_artist, prep_title);
            g_free(title_clean);
            curl_free(prep_artist);
            curl_free(prep_title);
        }
        g_free(artist_clean);
    }
    return result;
}

/////////////////////////////////

#define BEG "<img src=\"http://www.lyricsvip.com/images/phone2.gif\" alt=\"phone\" /></div>"
#define END "<br />\n<div class=\"ad\">"

static GList *lyrics_lyricsvip_parse(cb_object *capo)
{
    gchar *start = NULL;
    gchar *end = NULL;
    gchar *content = NULL;
    GList *result_list  = NULL;

    if((start = strstr(capo->cache->data, BEG)) != NULL) {
        if((end = strstr(start, END)) != NULL) {
            if(ABS(end - start) > 0) {
                * (end) = 0;

                content = strreplace(start, "<br />", "");
                if(content) {
                    GlyrMemCache *result = DL_init();
                    result->data = content;
                    result->size = strlen(content);
                    result_list = g_list_prepend(result_list, result);
                }
            }
        }
    }
    return result_list;
}

/////////////////////////////////

MetaDataSource lyrics_lyricsvip_src = {
    .name = "lyricsvip",
    .key  = 'v',
    .parser    = lyrics_lyricsvip_parse,
    .get_url   = lyrics_lyricsvip_url,
    .type      = GLYR_GET_LYRICS,
    .quality   = 60,
    .speed     = 85,
    .free_url  = true
};
