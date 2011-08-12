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
#include "../../core.h"
#include "../../stringlib.h"

#define LV_URL "http://www.lyricsvip.com/%s/%s-Lyrics.html"

const char * lyrics_lyricsvip_url(GlyrQuery * settings)
{
    char * result = NULL;
    char * artist_clean = strreplace(settings->artist, " ", "-");
    if(artist_clean)
    {
        char * title_clean =  strreplace(settings->title, " ", "-");
        if(title_clean)
        {
            result = g_strdup_printf(LV_URL, artist_clean, title_clean);
            g_free(title_clean);
        }
        g_free(artist_clean);
    }
    return result;
}

GList * lyrics_lyricsvip_parse(cb_object *capo)
{
    char * start = NULL;
    char * end = NULL;
    char * content = NULL;

    GlyrMemCache * r_cache = NULL;
    GList * r_list  = NULL;

    if ((start = strstr(capo->cache->data,"<table class=\"tbl0\">")) != NULL)
    {
        if ((end = strstr(start,"</table>")) != NULL)
        {
            if (ABS(end-start) > 0)
            {
                *(end) = 0;
                content = strreplace(start,"<br />",NULL);
                if(content)
                {
                    r_cache = DL_init();
                    r_cache->data = content;
                    r_cache->size = strlen(content);
                    r_cache->dsrc = strdup(capo->url);
                }
            }
        }
    }

    if(r_cache)
    {
        r_list = g_list_prepend(r_list,r_cache);
    }
    return r_list;
}

/*--------------------------------------------------------*/

MetaDataSource lyrics_lyricsvip_src =
{
    .name = "lyricsvip",
    .key  = 'v',
    .parser    = lyrics_lyricsvip_parse,
    .get_url   = lyrics_lyricsvip_url,
    .type      = GET_LYRICS,
    .endmarker = "</td></tr></table>",
    .quality   = 60,
    .speed     = 50,
    .free_url  = true
};
