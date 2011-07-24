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

#include "../core.h"
#include "../stringlib.h"
#include "../glyr.h"

const char * cover_allmusic_url(GlyQuery * s)
{
    if(s->cover.min_size < 200)
        return "http://www.allmusic.com/search/album/%album%";

    return NULL;
}

// begin of search results
#define SEARCH_TREE_BEGIN "<table class=\"search-results\""

// sole search result
#define SEARCH_NODE "<td><a href=\""
#define SEARCH_DELM "\">"

// artist
#define ARTIST_PART "<td>"
#define ARTIST_END  "</td>"
#define IMG_BEGIN "<div class=\"image\"> <img src=\""
#define IMG_ENDIN "\" alt=\""

GlyMemCache * parse_cover_page(GlyMemCache * dl)
{
    GlyMemCache * rc = NULL;
    if(dl != NULL)
    {
        char * img_begin = strstr(dl->data,IMG_BEGIN);
        if(img_begin != NULL)
        {
            char * img_url = copy_value(img_begin + strlen(IMG_BEGIN), strstr(img_begin,IMG_ENDIN));
            if(img_url != NULL)
            {
                if(strcmp(img_url,"/img/pages/site/icons/no_cover_200.gif"))
                {
                    rc = DL_init();
                    rc->data = img_url;
                    rc->size = strlen(img_url);
                }
                else
                {
                    free(img_url);
                    img_url=NULL;
                }
            }
        }
    }
    return rc;
}

GlyCacheList * cover_allmusic_parse(cb_object * capo)
{
    GlyCacheList * r_list = NULL;
    if( strstr(capo->cache->data, "<a href=\"\">Title</a></th>") )
    {
        GlyMemCache * result = parse_cover_page(capo->cache);
        r_list = DL_new_lst();
        DL_add_to_list(r_list, result);
        return r_list;
    }

    char * search_begin = NULL;
    if( (search_begin = strstr(capo->cache->data, SEARCH_TREE_BEGIN)) == NULL)
    {
        return NULL;
    }

    int urlc = 0;
    char *  node = search_begin;
    while( (node = strstr(node+1,SEARCH_NODE)) && continue_search(urlc,capo->s))
    {
        char * url = copy_value(node+strlen(SEARCH_NODE),strstr(node,SEARCH_DELM));
        if(url != NULL)
        {
            // We have the URL - now check the artist to be the one
            char * rr = strstr(node+1,ARTIST_PART);
            if(rr != NULL)
            {
                char * artist = copy_value(rr + strlen(ARTIST_PART), strstr(rr,ARTIST_END));
                if(artist != NULL)
                {
                    char * orig_artist = strdup(capo->s->artist);
                    if(orig_artist)
                    {
                        if(levenshtein_strcmp(ascii_strdown_modify(orig_artist),ascii_strdown_modify(artist)) <= capo->s->fuzzyness)
                        {
                            GlyMemCache * dl = download_single(url,capo->s,"<div class=\"artist\">");
                            if(dl != NULL)
                            {
                                GlyMemCache * result = parse_cover_page(dl);
                                if(result != NULL && result->data)
                                {
                                    if(!r_list) r_list = DL_new_lst();
                                    DL_add_to_list(r_list,result);
                                    urlc++;
                                }
                                DL_free(dl);
                            }
                        }
                        free(orig_artist);
                        orig_artist=NULL;
                    }
                }
                free(artist);
                artist=NULL;
            }
            free(url);
            url=NULL;
        }
    }
    return r_list;
}

MetaDataSource cover_allmusic_src = {
	.name      = "allmusic",
	.key       = 'm',
	.parser    = cover_allmusic_parse,
	.get_url   = cover_allmusic_url,
	.type      = GET_COVERART,
	.endmarker = NULL,
	.free_url  = false
};
