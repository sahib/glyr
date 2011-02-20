#define _GNU_SOURCE

// standard
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// own header
#include "metrolyrics.h"

// extended string lib
#include "../core.h"
#include "../core.h"
#include "../stringop.h"

// Search URL
#define ML_URL "http://www.metrolyrics.com/search.php?search=%artist%+%title%&category=artisttitle"
#define LEVEN_TOLERANCE 4
#define MAX_TRIES 5

// Just return URL
const char * lyrics_metrolyrics_url(glyr_settings_t * settings)
{
    return ML_URL;
}

static memCache_t * parse_lyrics_page(const char * buffer)
{
    memCache_t * result = NULL;
    if(buffer)
    {
        char * begin = strstr(buffer,"<div id=\"lyrics\">");
        if(begin)
        {
            char * end = strstr(begin,"</div>");
            if(end)
            {
                char * lyr = copy_value(begin,end);
                if(lyr)
                {
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

static bool approve_content(char * content, const char * compare)
{
    bool result = false;
    char * plain = beautify_lyrics(content); // cheap & well working ;-)
    if(plain)
    {
        char * tmp = strdup(compare);
        if(tmp)
        {
            if(levenshtein_strcmp(ascii_strdown_modify(plain,-1),ascii_strdown_modify(tmp,-1)) <= LEVEN_TOLERANCE)
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

cache_list * lyrics_metrolyrics_parse(cb_object * capo)
{
    memCache_t * result = NULL;
    cache_list * r_list = NULL;

    char * root = strstr(capo->cache->data,ROOT_NODE);
    if(root)
    {
        size_t tries = 0;
        char * node = root;
        while(node && (node = strstr(node+1,NODE_BEGIN)) && (tries++) < MAX_TRIES )
        {
            char * title_beg = strstr(node,NODE_ENDIN);
            if(title_beg)
            {
                char * title_end = strstr(title_beg,TITLE_END);
                if(title_end)
                {
                    char * title = copy_value(title_beg+strlen(NODE_ENDIN),title_end);
                    if(title)
                    {
                        if(approve_content(title,capo->s->title))
                        {
                            char * url = copy_value(node+strlen(NODE_BEGIN),title_beg);
                            if(url)
                            {
                                char * dl_url = strdup_printf("www.metrolyrics.com%s",url);
                                if(dl_url)
                                {
                                    memCache_t * dl_cache = download_single(dl_url,capo->s);
                                    if(dl_cache)
                                    {
                                        result = parse_lyrics_page(dl_cache->data);
                                        if(result)
                                        {
                                            result->dsrc = strdup(dl_url);
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
    if(result)
    {
    	r_list = DL_new_lst();
	DL_add_to_list(r_list,result);
    }
    return r_list;
}
