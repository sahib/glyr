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
#include "../stringop.h"
#include "../core.h"
#include "../glyr.h"

const char * ainfo_allmusic_url(GlyQuery * s)
{
    return "http://www.allmusic.com/search/artist/%artist%";
}

#define IMG_BEGIN "<p class=\"text\">"
#define IMG_ENDIN "</p>"

GlyMemCache * parse_bio_short(GlyMemCache * to_parse)
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
                    char * null_marker = strstr(cr,"&hellip;&nbsp;&nbsp;");
                    if(null_marker != NULL)
                    {
                        cr[null_marker - cr + 0] = '.';
                        cr[null_marker - cr + 1] = '.';
                        cr[null_marker - cr + 2] = '.';
                        cr[null_marker - cr + 3] =   0;
                    }

                    rche = DL_init();
                    rche->data = strip_html_unicode(cr);
                    rche->size = strlen(rche->data);
                    if(rche->data)
                        puts(rche->data);

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

#define ROOT "<div id=\"tabs\">"
#define ROOT_URL "http://www.allmusic.com/artist/"

GlyMemCache * parse_bio_long(GlyMemCache * to_parse, GlyQuery * s)
{
    GlyMemCache * rche = NULL;
    char * root = strstr(to_parse->data,ROOT);
    if(root)
    {
        char * url_root = strstr(root,ROOT_URL);
        if(url_root != NULL)
        {
            char * id = copy_value(url_root + strlen(ROOT_URL),strstr(url_root,"\">"));
            if(id != NULL)
            {
                char * url = strdup_printf(ROOT_URL"%s",id);
                if(url != NULL)
                {
                    GlyMemCache * dl = download_single(url,s,NULL);
                    if(dl != NULL)
                    {
                        rche = parse_bio_short(dl);
                        DL_free(dl);
                        dl=NULL;
                    }
                    free(url);
                    url=NULL;
                }
                free(id);
                id=NULL;
            }
        }
    }
    return rche;
}

// begin of search results
#define SEARCH_TREE_BEGIN "<table class=\"search-results\""

#define SEARCH_NODE "<td><a href=\""
#define SEARCH_DELM "\">"

GlyCacheList * ainfo_allmusic_parse(cb_object * capo)
{
    GlyCacheList * r_list = NULL;
    if( strstr(capo->cache->data, "<!--Begin Biography -->") )
    {
        r_list = DL_new_lst();
        GlyMemCache * info_short = parse_bio_short(capo->cache);
        GlyMemCache * info_long  = parse_bio_long (capo->cache,capo->s);

        DL_add_to_list(r_list,info_short);
        DL_add_to_list(r_list,info_long );
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
            GlyMemCache * dl = download_single(url,capo->s,NULL);
            if(dl != NULL)
            {
                if(!r_list) r_list = DL_new_lst();
                GlyMemCache * info_short = parse_bio_short(dl);
                if(info_short != NULL)
                {
                    DL_add_to_list(r_list,info_short);
                }

                GlyMemCache * info_long  = parse_bio_long (dl,capo->s);
                if(info_long != NULL)
                {
                    DL_add_to_list(r_list,info_long );
                }

                DL_free(dl);

                if(info_short || info_long)
                    urlc++;
            }
            free(url);
            url=NULL;
        }
    }
    return r_list;
}
