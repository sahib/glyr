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
#define ML_URL "http://www.metrolyrics.com/search.php?search=${artist}+${title}&category=artisttitle"
#define MAX_TRIES 5

// Just return URL
static const gchar * lyrics_metrolyrics_url(GlyrQuery * settings)
{
    return ML_URL;
}

/*--------------------------------------------------------*/

static void replace_from_message_inline(gchar * text)
{
    if(text != NULL)
    {
        gchar * from_msg_start = strstr(text,"[ From: ");
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

/*--------------------------------------------------------*/
#define LYRICS_DIV "<div id=\"lyrics\">"
#define LYRICS_END "</div>"

static GlyrMemCache * parse_lyrics_page(const gchar * buffer)
{
    GlyrMemCache * result = NULL;
    if(buffer != NULL)
    {
        gchar * begin = strstr(buffer,LYRICS_DIV);
        if(begin != NULL)
        {
            gchar * end = strstr(begin,LYRICS_END);
            if(end != NULL)
            {
                gchar * lyr = copy_value(begin,end);
                if(lyr != NULL)
                {
                    result = DL_init();
                    replace_from_message_inline(lyr);
                    result->data = lyr;
                    result->size = strlen(result->data);
                }
            }
        }
    }
    return result;
}

/*--------------------------------------------------------*/

#define ROOT_NODE "<div id=\"listResults\">"
#define NODE_BEGIN "<a href=\""
#define NODE_ENDIN "\" title=\""
#define TITLE_END  " Lyrics"
#define NEXT_NODE "<li>"
#define PAGES "<ul id=\"pages\">"

static GList * lyrics_metrolyrics_parse(cb_object * capo)
{
    GList * result_list = NULL;
    gchar * root = strstr(capo->cache->data,ROOT_NODE);
    if(root != NULL)
    {
        gsize tries = 0;
        gchar * node = root;
        while(continue_search(tries,capo->s) && (node = strstr(node+1,NODE_BEGIN)) && tries++ < MAX_TRIES)
        {
            gchar * title_beg = strstr(node,NODE_ENDIN);
            if(title_beg != NULL)
            {
                gchar * title_end = strstr(title_beg,TITLE_END);
                if(title_end != NULL)
                {
		    gsize node_begin_len = (sizeof NODE_BEGIN) - 1;
                    gchar * title = copy_value(title_beg + node_begin_len,title_end);
                    if(title != NULL)
                    {
			if(levenshtein_strnormcmp(capo->s,capo->s->title,title) <= capo->s->fuzzyness)
                        {
		  	    gsize node_endin_len = (sizeof NODE_ENDIN) - 1;
                            gchar * url = copy_value(node + node_endin_len, title_beg);
                            if(url != NULL)
                            {
                                gchar * dl_url = g_strdup_printf("www.metrolyrics.com%s",url);
                                GlyrMemCache * dl_cache = download_single(dl_url,capo->s,NULL);
                                if(dl_cache)
                                {
                                    GlyrMemCache * result = parse_lyrics_page(dl_cache->data);
                                    if(result)
                                    {
					    result->dsrc = g_strdup(dl_url);
					    result_list = g_list_prepend(result_list,result);
				    }
				    DL_free(dl_cache);
				}

				g_free(dl_url);
				g_free(url);
			    }
			}
			g_free(title);
		    }
		}
	    }
	    /* check if we accidentally reached end of results */
	    gchar * dist = strstr(node,PAGES);

	    /* hop to next node */
	    node = strstr(title_beg,NEXT_NODE);

	    /* Only advertisment behind dist */
	    if(node > dist) break;
	}
    }
    return result_list;
}

/*--------------------------------------------------------*/

MetaDataSource lyrics_metrolyrics_src =
{
	.name = "metrolyrics",
	.key  = 'm',
	.parser    = lyrics_metrolyrics_parse,
	.get_url   = lyrics_metrolyrics_url,
	.type      = GLYR_GET_LYRICS,
	.quality   = 35,
	.speed     = 40,
	.endmarker = NULL,
	.free_url  = false
};
