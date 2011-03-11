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
#include <stdlib.h>
#include <string.h>

#include "songlyrics.h"

#include "../core.h"
#include "../core.h"
#include "../stringop.h"

const char * lyrics_songlyrics_url(GlyQuery * settings)
{
    char * url = NULL;
    if(settings && settings->artist && settings->title)
    {
        char * esc_a = ascii_strdown(settings->artist);
        if(esc_a)
        {
            char * esc_t = ascii_strdown(settings->title);
            if(esc_t)
            {
                char * rep_a = strreplace(esc_a," ","-");
                if(rep_a)
                {
                    char * rep_t = strreplace(esc_t," ","-");
                    if(rep_t)
                    {
                        url = strdup_printf("http://www.songlyrics.com/%s/%s-lyrics/",rep_a,rep_t);
                        free(rep_t);
                    }
                    free(rep_a);
                }
                free(esc_t);
            }
            free(esc_a);
        }
    }
    return url;
}

GlyCacheList * lyrics_songlyrics_parse(cb_object * capo)
{
    GlyMemCache * result = NULL;
    GlyCacheList * r_list = NULL;
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
                    free(buf);
                }
            }
            else result = DL_error(NO_ENDIN_TAG);
        }
        else result = DL_error(NO_BEGIN_TAG);
    }
    if(result)
    {
        r_list = DL_new_lst();
        DL_add_to_list(r_list, result);
    }
    return r_list;
}
