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

#define INFO_BEGIN "</div><div style=\"text-align:center;\""
#define INFO_ENDIN " <a href=\""

const char * lyrics_lyricsreg_url(GlyrQuery * s)
{
    return "http://www.lyricsreg.com/lyrics/%artist%/%title%/";
}

GList * lyrics_lyricsreg_parse(cb_object * capo)
{
    GList * ls = NULL;
    size_t info_s_len = strlen(INFO_BEGIN);

    char * ptr = strstr(capo->cache->data, INFO_BEGIN);
    if(ptr != NULL)
    {
        ptr += info_s_len;
        char * end = strstr(ptr,INFO_ENDIN);
        if(end != NULL)
        {
            char * text_seg = copy_value(ptr,end);
            if(text_seg != NULL)
            {
                char * no_br_tags = strreplace(text_seg,"<br />",NULL);
                if(no_br_tags)
                {
                    GlyrMemCache * tmp = DL_init();
                    tmp->data = beautify_lyrics(no_br_tags);
                    tmp->size = tmp->data ? strlen(tmp->data) : 0;
                    tmp->dsrc = strdup(capo->url);
                    ls = g_list_prepend(ls,tmp);
                    g_free(no_br_tags);
                }
                g_free(text_seg);
            }
        }
    }
    return ls;
}

/*---------------------------------------------------*/

MetaDataSource lyrics_lyricsreg_src =
{
    .name = "lyricsreg",
    .key  = 'r',
    .parser    = lyrics_lyricsreg_parse,
    .get_url   = lyrics_lyricsreg_url,
    .type      = GET_LYRICS,
    .quality   = 42,
    .speed     = 50,
    .endmarker = NULL,
    .free_url  = false
};
