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
#include "../../core.h"
#include "../../stringlib.h"

// Search URL
#define ML_URL "http://www.metrolyrics.com/search.php?search=%artist%+%title%&category=artisttitle"
#define MAX_TRIES 5

// Just return URL
const char * lyrics_metrolyrics_url(GlyrQuery * settings)
{
    return ML_URL;
}

static void replace_from_message_inline(char * text)
{
	if(text != NULL)
	{
		char * from_msg_start = strstr(text,"[ From: ");
		if(from_msg_start != NULL)
		{
			while(from_msg_start[0] != '\n' && from_msg_start[0])
			{
				from_msg_start[0] = ' ';
				from_msg_start++;
			}

			if(from_msg_start[0] == '\n')
			{
				from_msg_start[0] = ' ';
			}
		}
	}
}

static GlyrMemCache * parse_lyrics_page(const char * buffer)
{
    GlyrMemCache * result = NULL;
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
		    replace_from_message_inline(lyr);
                    result->data = strreplace(lyr,"</span>\n","");
                    result->size = ABS(end-begin);
                    g_free(lyr);
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
    if(plain)
    {
        char * tmp = strdup(compare);
        if(tmp)
        {
            if(levenshtein_strcasecmp(plain,tmp) <= fuzz)
                result = true;

            g_free(tmp);
        }
        g_free(plain);
    }

    return result;
}

#define ROOT_NODE "<div id=\"listResults\">"
#define NODE_BEGIN "<a href=\""
#define NODE_ENDIN "\" title=\""
#define TITLE_END  " Lyrics"

GList * lyrics_metrolyrics_parse(cb_object * capo)
{
    GList * r_list = NULL;
    char * root = strstr(capo->cache->data,ROOT_NODE);
    if(root)
    {
        size_t tries = 0;
        char * node = root;
        while(node && (node = strstr(node+1,NODE_BEGIN)) && (tries++) < MAX_TRIES && continue_search(tries,capo->s))
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
                        if(approve_content(title,capo->s->title,capo->s->fuzzyness))
                        {
                            char * url = copy_value(node+strlen(NODE_BEGIN),title_beg);
                            if(url)
                            {
                                char * dl_url = g_strdup_printf("www.metrolyrics.com%s",url);
                                if(dl_url)
                                {
                                    GlyrMemCache * dl_cache = download_single(dl_url,capo->s,NULL);
                                    if(dl_cache)
                                    {
                                        GlyrMemCache * result = parse_lyrics_page(dl_cache->data);
                                        if(result)
                                        {
                                            result->dsrc = strdup(dl_url);
                                            r_list = g_list_prepend(r_list,result);
                                        }
                                        DL_free(dl_cache);
                                    }
                                    g_free(dl_url);
                                }
                                g_free(url);
                            }
                        }
                        g_free(title);
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

/*--------------------------------------------------------*/

MetaDataSource lyrics_metrolyrics_src =
{
    .name = "metrolyrics",
    .key  = 'm',
    .parser    = lyrics_metrolyrics_parse,
    .get_url   = lyrics_metrolyrics_url,
    .type      = GET_LYRICS,
    .quality   = 25,
    .speed     = 40,
    .endmarker = NULL,
    .free_url  = false
};
