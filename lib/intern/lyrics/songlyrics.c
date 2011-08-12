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

const char * lyrics_songlyrics_url(GlyQuery * settings)
{
    char * url = NULL;
    if(settings && settings->artist && settings->title)
    {
        gchar * esc_a = g_utf8_strdown(settings->artist,-1);
        if(esc_a != NULL)
        {
            char * esc_t = g_utf8_strdown(settings->title,-1);
            if(esc_t != NULL)
            {
                char * rep_a = strreplace(esc_a," ","-");
                if(rep_a != NULL)
                {
                    char * rep_t = strreplace(esc_t," ","-");
                    if(rep_t != NULL)
                    {
                        url = g_strdup_printf("http://www.songlyrics.com/%s/%s-lyrics/",rep_a,rep_t);
                        g_free(rep_t);
                    }
                    g_free(rep_a);
                }
                g_free(esc_t);
            }
            g_free(esc_a);
        }
    }
    return url;
}

GList * lyrics_songlyrics_parse(cb_object * capo)
{
    GlyMemCache * result = NULL;
    GList * r_list = NULL;
    char * find_tag, * find_end;

    if( (find_tag = strstr(capo->cache->data,"<p id=\"songLyricsDiv\"")) != NULL)
    {
        if( (find_tag = strstr(find_tag,"&#")) != NULL)
        {
            if( (find_end = strstr(find_tag,"<p id=\"songLyricsSmallDiv\"")) != NULL)
            {
                char * buf = copy_value(find_tag,find_end);
                if(buf)
                {
                    char * brd = strreplace(buf,"<br />","");
                    if(brd)
                    {
                        result = DL_init();
                        result->data = brd;
                        result->size = strlen(brd);
                        result->dsrc = strdup(capo->url);
                    }
                    g_free(buf);
                }
            }
        }
    }
    if(result)
    {
        r_list = g_list_prepend(r_list, result);
    }
    return r_list;
}

/*--------------------------------------------------------*/

MetaDataSource lyrics_songlyrics_src =
{
    .name = "songlyrics",
    .key  = 's',
    .parser    = lyrics_songlyrics_parse,
    .get_url   = lyrics_songlyrics_url,
    .type      = GET_LYRICS,
    .quality   = 20,
    .speed     = 30,
    .endmarker = NULL,
    .free_url  = true
};
