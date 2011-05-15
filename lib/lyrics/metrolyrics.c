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

// standard
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// own header
#include "metrolyrics.h"

// extended string lib
#include "../core.h"
#include "../stringlib.h"

// Search URL
#define ML_URL "http://www.metrolyrics.com/search.php?search=%artist%+%title%&category=artisttitle"
#define MAX_TRIES 5

// Just return URL
const char * lyrics_metrolyrics_url(GlyQuery * settings)
{
        return ML_URL;
}

static GlyMemCache * parse_lyrics_page(const char * buffer)
{
        GlyMemCache * result = NULL;
        if(buffer) {
                char * begin = strstr(buffer,"<div id=\"lyrics\">");
                if(begin) {
                        char * end = strstr(begin,"</div>");
                        if(end) {
                                char * lyr = copy_value(begin,end);
                                if(lyr) {
                                        result = DL_init();
                                        result->data = strreplace(lyr,"<br />","");
                                        result->size = ABS(end-begin);
                                        free(lyr);
                                }
                        }
                }
        }
        return result;
}

static bool approve_content(char * content, const char * compare, size_t fuzz)
{
        bool result = false;
        char * plain = beautify_lyrics(content); // cheap & well working ;-)
        if(plain) {
                char * tmp = strdup(compare);
                if(tmp) {
                        if(levenshtein_strcmp(ascii_strdown_modify(plain),ascii_strdown_modify(tmp)) <= fuzz)
                                result = true;

                        free(tmp);
                }
                free(plain);
        }

        return result;
}

#define ROOT_NODE "<div id=\"listResults\">"
#define NODE_BEGIN "<a href=\""
#define NODE_ENDIN "\" title=\""
#define TITLE_END  " Lyrics"

GlyCacheList * lyrics_metrolyrics_parse(cb_object * capo)
{
        GlyCacheList * r_list = NULL;
        char * root = strstr(capo->cache->data,ROOT_NODE);
        if(root) {
                size_t tries = 0;
                char * node = root;
                while(node && (node = strstr(node+1,NODE_BEGIN)) && (tries++) < MAX_TRIES && continue_search(tries,capo->s)) {
                        char * title_beg = strstr(node,NODE_ENDIN);
                        if(title_beg) {
                                char * title_end = strstr(title_beg,TITLE_END);
                                if(title_end) {
                                        char * title = copy_value(title_beg+strlen(NODE_ENDIN),title_end);
                                        if(title) {
                                                if(approve_content(title,capo->s->title,capo->s->fuzzyness)) {
                                                        char * url = copy_value(node+strlen(NODE_BEGIN),title_beg);
                                                        if(url) {
                                                                char * dl_url = strdup_printf("www.metrolyrics.com%s",url);
                                                                if(dl_url) {
                                                                        GlyMemCache * dl_cache = download_single(dl_url,capo->s,NULL);
                                                                        if(dl_cache) {
                                                                                GlyMemCache * result = parse_lyrics_page(dl_cache->data);
                                                                                if(result) {
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
                                                free(title);
                                        }
                                }
                        }
                        // check if we accidentally reached end of results
                        char * dist = strstr(node,"<ul id=\"pages\">");

                        // hop to next node
                        node = strstr(title_beg,"<li>");

                        if(node > dist) break;
                }
        }
        return r_list;
}
