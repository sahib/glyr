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
#include <stdbool.h>
#include "lyricsvip.h"

#include "../core.h"
#include "../stringlib.h"

#define LV_URL "http://www.lyricsvip.com/%s/%s-Lyrics.html"

const char * lyrics_lyricsvip_url(GlyQuery * settings)
{
    char * result = NULL;
    char * artist_clean = strreplace(settings->artist, " ", "-");
    if(artist_clean)
    {
        char * title_clean =  strreplace(settings->title, " ", "-");
        if(title_clean)
        {
            result = strdup_printf(LV_URL, artist_clean, title_clean);
            free(title_clean);
        }
        free(artist_clean);
    }
    return result;
}

GlyCacheList * lyrics_lyricsvip_parse(cb_object *capo)
{
    char * start = NULL;
    char * end = NULL;
    char * content = NULL;

    GlyMemCache * r_cache = NULL;
    GlyCacheList * r_list  = NULL;

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
        else r_cache = DL_error(NO_ENDIN_TAG);
    }
    else r_cache = DL_error(NO_BEGIN_TAG);

    if(r_cache)
    {
        r_list = DL_new_lst();
        DL_add_to_list(r_list,r_cache);
    }
    return r_list;
}

