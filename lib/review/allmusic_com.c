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
#include <string.h>

#include "allmusic_com.h"

#include "../stringop.h"
#include "../core.h"

// begin of search results
#define SEARCH_TREE_BEGIN "<table class=\"search-results\""

// sole search result
#define SEARCH_NODE "<td><a href=\""
#define SEARCH_DELM "\">"

// artist
#define ARTIST_PART "<td>"
#define ARTIST_END  "</td>"

#define LV_MAX 3

const char * review_allmusic_url(GlyQuery * s)
{
    return "http://www.allmusic.com/search/album/%album%";
}

#define IMG_BEGIN "<p class=\"text\">"
#define IMG_ENDIN "</p>"

GlyMemCache * parse_text(GlyMemCache * to_parse)
{
    GlyMemCache * rche = NULL;
    char * text_begin = strstr(to_parse->data,IMG_BEGIN);
    if(text_begin != NULL)
    {
        char * text_endin = strstr(text_begin,IMG_ENDIN);
        if(text_endin != NULL)
        {
            char * text = copy_value(text_begin + strlen(IMG_BEGIN),text_endin);
            if(text != NULL)
            {
                char * cr = remove_html_tags_from_string(text,strlen(text));
                if(cr != NULL)
                {
                    rche = DL_init();
                    rche->data = strip_html_unicode(cr);
                    rche->size = strlen(rche->data);

                    free(cr);
                    cr = NULL;
                }
                free(text);
                text = NULL;
            }
        }
    }
    return rche;
}


GlyCacheList * review_allmusic_parse(cb_object * capo)
{
    GlyCacheList * r_list = NULL;
    if( strstr(capo->cache->data, "<a href=\"\">Title</a></th>") )
    {
        GlyMemCache * result = parse_text(capo->cache);
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
                        ascii_strdown_modify(artist,-1);
                        ascii_strdown_modify(orig_artist,-1);
                        if(levenshtein_strcmp(orig_artist,artist) <= LV_MAX)
                        {
                            char * review_url = strdup_printf("%s/review",url);
                            if(review_url)
                            {
                                GlyMemCache * dl = download_single(review_url,capo->s,NULL);
                                if(dl != NULL)
                                {
                                    GlyMemCache * result = parse_text(dl);
                                    if(result != NULL)
                                    {
                                        if(!r_list) r_list = DL_new_lst();
                                        DL_add_to_list(r_list,result);
                                        urlc++;
                                    }
                                    DL_free(dl);
                                }
                                free(review_url);
                                review_url = NULL;
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
