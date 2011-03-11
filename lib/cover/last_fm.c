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

#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include "last_fm.h"

#include "../cover.h"
#include "../stringop.h"
#include "../core.h"

#define API_KEY API_KEY_LASTFM

const char * cover_lastfm_url(GlyQuery * sets)
{
    if(sets->cover.min_size <= 350 || sets->cover.min_size == -1)
    {
        return "http://ws.audioscrobbler.com/2.0/?method=album.search&album=%artist%+%album%&api_key="API_KEY;
    }
    return NULL;
}

GlyCacheList * cover_lastfm_parse(cb_object *capo)
{
    // Handle size requirements (Default to large)
    const char *tag_ssize = NULL ;
    const char *tag_esize = "</image>";

    // find desired size
    if( size_is_okay(300,capo->s->cover.min_size,capo->s->cover.max_size) )
        tag_ssize = "<image size=\"extralarge\">";
    else if( size_is_okay(125,capo->s->cover.min_size,capo->s->cover.max_size) )
        tag_ssize = "<image size=\"large\">";
    else if( size_is_okay(64, capo->s->cover.min_size,capo->s->cover.max_size) )
        tag_ssize = "<image size=\"middle\">";
    else if( size_is_okay(34, capo->s->cover.min_size,capo->s->cover.max_size) )
        tag_ssize = "<image size=\"small\">";
    else if ( true || false )
        tag_ssize = "<image size=\"extralarge\">";

    // The (perhaps) result
    GlyMemCache * result = NULL;
    GlyCacheList * r_list = NULL;

    int urlc = 0;

    char * find = capo->cache->data;
    while( (find = strstr(find+1, tag_ssize)) != NULL && continue_search(urlc,capo->s))
    {
        char * end_tag = NULL;
        if( (end_tag = strstr(find, tag_esize)) != NULL)
        {
            char * url = NULL;
            if( (url = copy_value(find + strlen(tag_ssize), end_tag)) != NULL)
            {
                if(strcmp(url,"http://cdn.last.fm/flatness/catalogue/noimage/2/default_album_medium.png"))
                {
                    if(!r_list) r_list = DL_new_lst();

                    result = DL_init();
                    result->data = url;
                    result->size = end_tag - (find + strlen(tag_ssize));

                    DL_add_to_list(r_list,result);
                    urlc++;
                }
                else
                {
                    free(url);
                    url=NULL;
                }
            }
        }
    }
    return r_list;
}
