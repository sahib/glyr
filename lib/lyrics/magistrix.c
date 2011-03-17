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
#include "magistrix.h"
#include "../core.h"
#include "../stringop.h"

#define MG_URL "http://www.magistrix.de/lyrics/search?q=%artist%+%title%"

const char * lyrics_magistrix_url(GlyQuery * settings)
{
    return MG_URL;
}

static GlyMemCache * parse_lyric_page(const char * buffer)
{
    GlyMemCache * result = NULL;
    if(buffer)
    {
        char * begin = strstr(buffer,"<div id='songtext'>");
        if(begin)
        {
            begin = strstr(begin,"</div>");
            {
                char * end = strstr(begin+1,"</div>");
                if(end)
                {
                    char * lyr = copy_value(begin,end);
                    if(lyr)
                    {
                        result = DL_init();
                        result->data = strreplace(lyr,"<br />","");
                        result->size = strlen(result->data);
                        free(lyr);
                    }
                }
            }
        }
    }
    return result;
}

static bool approve_content(char * content, const char * compare, size_t fuzz)
{
    if(compare)
    {
        char * tmp = strdup(compare);
        if(levenshtein_strcmp(ascii_strdown_modify(content),ascii_strdown_modify(tmp)) <= fuzz)
        {
            free(tmp);
            return true;
        }
        free(tmp);
    }
    return false;
}

#define ARTIST_BEGIN "class=\"artistIcon bgMove\">"
#define TITLE_BEGIN "\" class=\"lyricIcon bgMove\">"
#define URL_BEGIN   "<a href=\""
#define SEARCH_END   "</a>"
#define MAX_TRIES 7

GlyCacheList * lyrics_magistrix_parse (cb_object * capo)
{
    GlyCacheList * r_list=NULL;
    if( strstr(capo->cache->data,"<div class='empty_collection'>") == NULL) // No songtext page?
    {
        if( strstr(capo->cache->data,"<title>Songtext-Suche</title>") == NULL) // Are we not on the search result page?
        {
            GlyMemCache * result = parse_lyric_page(capo->cache->data);
            if(result)
            {
                result->dsrc = strdup(capo->url);
                if(!r_list) r_list = DL_new_lst();
                DL_add_to_list(r_list,result);
            }
        }
        else
        {
            char * node = capo->cache->data;
            int ctr = 0, urlc = 0;
            while( (node = strstr(node+1,"<tr class='topLine'>")) && MAX_TRIES >= ctr && continue_search(urlc,capo->s))
            {
                ctr++;
                char * artist = copy_value(strstr(node,ARTIST_BEGIN)+strlen(ARTIST_BEGIN),strstr(node,"</a>"));
                if(artist)
                {
                    if(approve_content(artist,capo->s->artist,capo->s->fuzzyness))
                    {
                        char * title_begin = strstr(node,TITLE_BEGIN);
                        if(title_begin)
                        {
                            char * title = copy_value(title_begin+strlen(TITLE_BEGIN),strstr(title_begin,"</a>"));
                            if(title)
                            {
                                if(approve_content(title,capo->s->title,capo->s->fuzzyness))
                                {
                                    char * url_begin = strstr(node,URL_BEGIN);
                                    if(url_begin)
                                    {
                                        url_begin = strstr(url_begin+1,URL_BEGIN);
                                        if(url_begin)
                                        {
                                            char * url = copy_value(url_begin+strlen(URL_BEGIN),title_begin);
                                            if(url)
                                            {
                                                char * dl_url = strdup_printf("www.magistrix.de%s",url);
                                                if(dl_url)
                                                {
                                                    // We don't need the ugly comments
                                                    GlyMemCache * dl_cache = download_single(dl_url,capo->s,"<div class='comments'");
                                                    if(dl_cache)
                                                    {
                                                        GlyMemCache * result = parse_lyric_page(dl_cache->data);
                                                        if(result)
                                                        {
                                                            urlc++;
                                                            result->dsrc = strdup(dl_url);
                                                            if(!r_list) r_list = DL_new_lst();
                                                            DL_add_to_list(r_list,result);
                                                        }
                                                        DL_free(dl_cache);
                                                    }
                                                    free(dl_url);
                                                }
                                                free(url);
                                            }
                                        }
                                    }
                                }
                                free(title);
                            }
                        }
                    }
                    free(artist);
                }
            }
        }
    }
    return r_list;
}
