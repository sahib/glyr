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

#define MG_URL "http://www.magistrix.de/lyrics/search?q=${artist}+${title}"

static const char * lyrics_magistrix_url (GlyrQuery * settings)
{
    return MG_URL;
}

///////////////////////////////////

static GlyrMemCache * parse_lyric_page (GlyrMemCache * cache)
{
    GlyrMemCache * result = NULL;
    g_return_val_if_fail (cache,NULL);
    g_return_val_if_fail (cache->data,NULL);

    gchar * data = get_search_value (cache->data,"id='songtext'>","</div>");
    if (data != NULL)
    {
        result = DL_init();
        result->data = data;
        result->size = strlen (data);
        result->dsrc = g_strdup (cache->dsrc);
    }

    return result;
}

///////////////////////////////////

#define SEARCH_FIRST_RESULT "<table class='searchresult'>"
#define SEARCH_LAST_RESULT  "</table>"
#define SEARCH_NODE "<div class='title'>"
#define SEARCH_LINK_START "&ndash;\n<a href=\""
#define SEARCH_LINK_END   "\" class"

static GList * parse_search_result_page (cb_object * capo)
{
    GList * result_list = NULL;
    char * first_result = strstr (capo->cache->data, SEARCH_FIRST_RESULT);
    if (first_result != NULL)
    {
        char * end_of_results = strstr (first_result + sizeof (SEARCH_FIRST_RESULT), SEARCH_LAST_RESULT);
        if (end_of_results)
        {
            char * node = first_result;
            while ( (node = strstr (node + sizeof (SEARCH_NODE), SEARCH_NODE) )
                    && continue_search (g_list_length (result_list), capo->s) )
            {
                char * new_url = get_search_value (node, SEARCH_LINK_START, SEARCH_LINK_END);
                if (new_url != NULL)
                {
                    char * full_url = g_strdup_printf ("www.magistrix.de%s", new_url);
                    GlyrMemCache * lyrics_page = download_single (full_url, capo->s, NULL);
                    if (lyrics_page)
                    {
                        GlyrMemCache * item = parse_lyric_page (lyrics_page);
                        if (item != NULL)
                        {
                            result_list = g_list_prepend (result_list, item);
                        }
                        DL_free (lyrics_page);
                    }
                    g_free (new_url);
                    g_free (full_url);
                }
            }
        }
    }
    return result_list;
}

///////////////////////////////////

static GList * lyrics_magistrix_parse (cb_object * capo)
{
    GList * result_list = NULL;
    if (strstr (capo->cache->data,"Es wurden keine Songtexte gefunden") == NULL) /* "No songtext" page? */
    {
        if (strstr (capo->cache->data,"<title>Songtext-Suche</title>") == NULL) /* Are we not on the search result page? */
        {
            GlyrMemCache * result = parse_lyric_page (capo->cache);
            if (result != NULL)
            {
                result_list = g_list_prepend (result_list,result);
            }
        }
        else
        {
            /* Parse Searchresult page */
            result_list = parse_search_result_page (capo);
        }
    }
    return result_list;
}

///////////////////////////////////

MetaDataSource lyrics_magistrix_src =
{
    .name = "magistrix",
    .key  = 'x',
    .parser    = lyrics_magistrix_parse,
    .get_url   = lyrics_magistrix_url,
    .type      = GLYR_GET_LYRICS,
    .quality   = 60,
    .speed     = 75,
    .endmarker = NULL,
    .free_url  = false
};
