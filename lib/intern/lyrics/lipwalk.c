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

#define LIPWALK_URL "http://www.lipwalklyrics.com/component/lyrics/search/index.php?search=%artist%%20%title%"
#define START "<div id=\"lyric\">"
#define END "</div>"

const char * lyrics_lipwalk_url(GlyrQuery * settings)
{
    return LIPWALK_URL;
}

GList * lyrics_lipwalk_parse(cb_object *capo)
{
    char * start = NULL;
    char * end = NULL;
    char * content = NULL;

    GlyrMemCache * r_cache = NULL;
    GList * r_list  = NULL;

    if ((start = strstr(capo->cache->data,START)) != NULL)
    {
        if ((end = strstr(start,END)) != NULL)
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

/*---------------------------------------------------*/

MetaDataSource lyrics_lipwalk_src =
{
    .name = "lipwalk",
    .key  = 'z',
    .parser    = lyrics_lipwalk_parse,
    .get_url   = lyrics_lipwalk_url,
    .type      = GET_LYRICS,
    .quality   = 90,
    .speed     = 90,
    .endmarker = NULL,
    .free_url  = false
};
