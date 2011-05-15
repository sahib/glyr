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

#include "lyricstime.h"
#include "../core.h"
#include "../stringlib.h"

#define GET_URL "http://www.lyricstime.com/search/?q=%artist%+%title%&t=default"

/*--------------------------------------------------------*/

const char * lyrics_lyricstime_url(GlyQuery * settings)
{
        return GET_URL;
}

/*--------------------------------------------------------*/

#define LYR_BEGIN "<div id=\"songlyrics\" >"
#define LYR_ENDIN "</div>"

static GlyMemCache * parse_page(GlyMemCache * dl, cb_object * capo)
{
        GlyMemCache * result = NULL;
        if(dl != NULL) {
                char * begin = strstr(dl->data,LYR_BEGIN);
                if(begin != NULL) {
                        begin += strlen(LYR_BEGIN);
                        char * text = copy_value(begin,strstr(begin,LYR_ENDIN));
                        if(text != NULL) {
                                char * no_br_tags = strreplace(text,"<br />",NULL);
                                if(no_br_tags != NULL) {
                                        result = DL_init();
                                        result->data = beautify_lyrics(no_br_tags);
                                        result->size = (result->data) ? strlen(result->data) : 0;
                                        result->dsrc = strdup(capo->url);

                                        free(no_br_tags);
                                }
                                free(text);
                        }
                }
        }
        return result;
}

/*--------------------------------------------------------*/

#define START_SEARCH "<div id=\"searchresult\">"
#define SEARCH_ENDIN "</div>"
#define NODE_BEGIN   "<li><a href=\""
#define NODE_ENDIN   "\">"

#define SPAN_BEGIN "<span class"

GlyCacheList * lyrics_lyricstime_parse(cb_object * capo)
{
        GlyCacheList * rList = NULL;
        char * start = capo->cache->data;
        if(start != NULL) {
                char * div_end = strstr(start,SEARCH_ENDIN);
                char * node = capo->cache->data;
                char * backpointer = NULL;

                size_t ctr = 0, nlen = strlen(NODE_BEGIN);
                while( (node = strstr(node+1,NODE_BEGIN)) != NULL && continue_search(ctr,capo->s)) {
                        if(div_end >= node)
                                break;

                        bool i_shall_continue = false;
                        if(backpointer != NULL) {
                                char * span = strstr(backpointer,SPAN_BEGIN);
                                if(span != NULL) {
                                        char * artist_beg = strstr(span,"<b>");
                                        if(artist_beg) {
                                                char * artist_end = strstr(artist_beg,"</b>");
                                                if(artist_end) {
                                                        char * artist_val = copy_value(artist_beg+strlen("<b>"),artist_end);
                                                        if(artist_val != NULL) {
                                                                if(levenshtein_strcasecmp(artist_val,capo->s->artist) <= capo->s->fuzzyness)
                                                                        i_shall_continue = true;

                                                                free(artist_val);
                                                        }
                                                }
                                        }
                                }
                        } else i_shall_continue = true;

                        if(i_shall_continue) {
                                char * end_of_url = strstr(node+nlen,NODE_ENDIN);
                                if(end_of_url != NULL) {
                                        char * url = copy_value(node+nlen,end_of_url);
                                        if(url != NULL) {
                                                char * full_url = strdup_printf("http://www.lyricstime.com%s",url);
                                                GlyMemCache * dl_cache = download_single(full_url,capo->s,NULL);
                                                if(dl_cache) {
                                                        GlyMemCache * r_cache = parse_page(dl_cache,capo);
                                                        if(r_cache) {
                                                                if(!rList) rList = DL_new_lst();
                                                                DL_add_to_list(rList,r_cache);

                                                                ctr++;
                                                        }
                                                        DL_free(dl_cache);
                                                        free(full_url);
                                                }
                                                free(url);
                                        }
                                }
                        }
                        backpointer = node;
                }
        }
        return rList;
}

/*--------------------------------------------------------*/
