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

#include "flickr.h"
#include "../core.h"
#include "../stringop.h"

#define LINE_BEGIN "<photo id="
#define LINE_ENDIN "/>"

const char * photos_flickr_url(GlyQuery * settings)
{
    return strdup_printf("http://api.flickr.com/services/rest/"
                         "?method=flickr.photos.search&"
                         "api_key="API_KEY_FLICKR"&"
                         "tags=%s&"
                         "content_type=1&"
                         "media=photos&"
                         "is_gallery=true&"
                         "per_page=%d",
                         settings->artist,
                         settings->number
                        );
}

static char * get_field_by_name(const char * string, const char * name)
{
    if(string)
    {
        char * find = strstr(string,name);
        if(find)
        {
            find += strlen(name);

            if(*find == '"')
                find ++;

            char * end = strstr(find,"\"");
            if(end)
            {
                return copy_value(find,end);
            }
        }
    }
    return NULL;
}

GlyCacheList * photos_flickr_parse(cb_object * capo)
{
    // Needed: ID,secret,server,farm
    char * ph_begin = capo->cache->data;
    size_t urlc = 0;
    GlyCacheList * r_list = NULL;

    while( (ph_begin=strstr(ph_begin,LINE_BEGIN)) != NULL && continue_search(urlc,capo->s))
    {
        if(! *(++ph_begin))
            continue;

        char * ph_end = strstr(ph_begin,LINE_ENDIN);
        if(ph_end)
        {
            char * linebf = copy_value(ph_begin,ph_end);
            if(linebf)
            {
                char * ID = get_field_by_name(linebf, "id=");
                char * SC = get_field_by_name(linebf, "secret=");
                char * SV = get_field_by_name(linebf, "server=");
                char * FR = get_field_by_name(linebf, "farm=");
                free(linebf);
                linebf = NULL;

                if(!r_list) r_list = DL_new_lst();
                GlyMemCache * cache = DL_init();
                cache->data = strdup_printf("http://farm%s.static.flickr.com/%s/%s_%s.jpg",FR,SV,ID,SC);
                DL_add_to_list(r_list,cache);

                if(ID)
                    free(ID);
                if(SC)
                    free(SC);
                if(SV)
                    free(SV);
                if(FR)
                    free(FR);

                urlc++;
            }
        }
    }
    return r_list;
}
