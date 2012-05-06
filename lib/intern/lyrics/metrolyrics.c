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

///////////////////////////////////

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

///////////////////////////////////

#define LYRICS_DIV "<div id=\"lyrics-body\">"
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

///////////////////////////////////

//#define ROOT_NODE "<div id=\"listResults\">"
#define ROOT_NODE "<ul id=\"search-results\""
#define NODE_BEGIN "<a href=\""
#define END_OF_SEARCH "<li id=\"view-more-bar\">"

/*
 *  A single node:
	<a href="/friede-sei-mit-dir-lyrics-die-apokalyptischen-reiter.html">
		<span class="img"><img src="http://netstorage.metrolyrics.com/artists/face/maf.jpg" alt="Die Apokalyptischen Reiter" /></span>
		<span class="title">Die Apokalyptischen Reiter<br /><strong>Friede Sei Mit Dir Lyrics</strong></span>
	</a>

    artist = get_search_value("<span class=\"title\">","<br />");
    title  = get_search_value("<strong>","</strong>");
*/

static GList * lyrics_metrolyrics_parse(cb_object * capo)
{
    GList * result_list = NULL;
    gchar * root = strstr(capo->cache->data,ROOT_NODE);

    if(root != NULL)
    {
        gchar * end_of_earch = strstr(root,END_OF_SEARCH);
        gchar * node = root;

        gsize tries = 0;
        gsize nodelen = (sizeof NODE_BEGIN);

        while(continue_search(g_list_length(result_list),capo->s) && (node = strstr(node + nodelen,NODE_BEGIN)) && tries < MAX_TRIES)
        {
                node += nodelen;

                gchar * m_artist = get_search_value(node,"<span class=\"title\">","<br />");
                gchar * m_title  = get_search_value(node,"<strong>"," Lyrics</strong>");

                if(levenshtein_strnormcmp(capo->s,capo->s->title, m_title)  <= capo->s->fuzzyness &&
                   levenshtein_strnormcmp(capo->s,capo->s->artist,m_artist) <= capo->s->fuzzyness)
                {
                    gchar * relative_url = copy_value(node, strstr(node,"\">"));
                    if(relative_url != NULL)
                    {
                        gchar * page_url = g_strdup_printf("www.metrolyrics.com/%s",relative_url);

                        tries++;
                        GlyrMemCache * page_cache = download_single(page_url,capo->s,NULL);

                        if(page_cache != NULL)
                        {
                            GlyrMemCache * result = parse_lyrics_page(page_cache->data);
                            if(result != NULL)
                            {
                                result->dsrc = g_strdup(page_url);
                                result_list  = g_list_prepend(result_list,result);
                            }
                            DL_free(page_cache);
                        }
                        g_free(page_url);
                        g_free(relative_url);
                    }
                }
               
                g_free(m_artist);
                g_free(m_title); 

                /* Only advertisment behind dist */
                if(node >= end_of_earch) break;
        }
    }
    return result_list;
}

///////////////////////////////////

MetaDataSource lyrics_metrolyrics_src =
{
    .name = "metrolyrics",
    .key  = 'm',
    .parser    = lyrics_metrolyrics_parse,
    .get_url   = lyrics_metrolyrics_url,
    .type      = GLYR_GET_LYRICS,
    .quality   = 30,
    .speed     = 60,
    .endmarker = NULL,
    .free_url  = false
};
