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

#include "darklyrics.h"

#include "../core.h"
#include "../core.h"
#include "../stringop.h"

#define DARK_URL "http://darklyrics.com/lyrics/%artist%/%album%.html#1"

const char * lyrics_darklyrics_url(GlyQuery * settings)
{
    return DARK_URL;
}

GlyCacheList * lyrics_darklyrics_parse(cb_object * capo)
{
    /* We have to search for the title in the data,
     * Therefore we convert everything to lowercase
     * to increase success chances.
     * Afterwards we copy from cache->data so we have
     * the correct case in the finaly lyrics
     * */

    GlyCacheList * r_list = NULL;
    GlyMemCache * r_cache = NULL;

    char *searchstring = strdup_printf(". %s%s",capo->s->title,"</a></h3>");
    if(searchstring)
    {
        ascii_strdown_modify(searchstring,-1);
        char *cache_copy = strdup(capo->cache->data);
        if(cache_copy)
        {
            ascii_strdown_modify(cache_copy,-1);
            char *head = strstr(cache_copy,searchstring);
            if(head)
            {
                char *foot  = strstr(head, "<a name=");
                if(foot)
                {
                    char * html_code = copy_value(head,foot);
                    if(html_code)
                    {
                        r_cache = DL_init();
                        r_cache->data = strreplace(html_code,"<br />","");
                        r_cache->size = strlen(r_cache->data);
                        r_cache->dsrc = strdup(capo->url);
                        free(html_code);
                    }
                }
                else
                {
                    r_cache = DL_error(NO_ENDIN_TAG);
                }
            }
            free(cache_copy);
        }
        free(searchstring);
    }
    else
    {
        r_cache = DL_error(NO_BEGIN_TAG);
    }

    if(r_cache)
    {
        r_list = DL_new_lst();
        DL_add_to_list(r_list,r_cache);
    }
    return r_list;
}
