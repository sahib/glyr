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

#define GT_BASE "http://www.guitaretab.com"
#define GT_URL GT_BASE"/fetch/?type=tab&query=${title}"

/////////////////////////////////

static const gchar * gt_guitaretabs_url (GlyrQuery * settings)
{
    return GT_URL;
}

/////////////////////////////////

static GlyrMemCache * parse_single_page (GlyrQuery * s, const gchar * url)
{
    GlyrMemCache * result = NULL;
    GlyrMemCache * tab_cache = download_single (url,s,NULL);
    if (tab_cache != NULL)
    {
        gchar * content = get_search_value (tab_cache->data,"<pre>","</pre>");
        if (content != NULL)
        {
            result = DL_init();
            result->data = content;
            result->size = strlen (content);
            result->dsrc = g_strdup (url);
        }
        DL_free (tab_cache);
    }
    return result;
}

/////////////////////////////////

#define SEARCH_RESULTS_BEGIN "<div class=\"specrows albums\">"
#define SEARCH_RESULTS_ENDIN "</table>"
#define SEARCH_NODE "<a href=\""

#define ARTIST_BEGIN "\">"
#define ARTIST_END "</a>"
#define URL_END "\" "

#define TITLE_BEGIN "\">"
#define TITLE_ENDIN "</a></li>"

static GList * gt_guitaretabs_parse (cb_object * capo)
{
    GList * result_list = NULL;
    gchar * begin_search = strstr (capo->cache->data,SEARCH_RESULTS_BEGIN);
    if (begin_search != NULL)
    {
        /* End need to assure we don't get over the search results */
        gchar * endin_search = strstr (begin_search,SEARCH_RESULTS_ENDIN);
        if (endin_search != NULL)
        {
            /* Go through all search results */
            gchar * node  = begin_search;
            gsize nodelen = (sizeof SEARCH_NODE) - 1;
            while (continue_search (g_list_length (result_list),capo->s) && (node = strstr (node + nodelen, SEARCH_NODE) ) != NULL && node <= endin_search)
            {
                gchar * artist = get_search_value (node,ARTIST_BEGIN,ARTIST_END);
                node = strstr (node + nodelen, SEARCH_NODE);
                if (node != NULL)
                {
                    gchar * url = get_search_value (node,SEARCH_NODE,URL_END);
                    gchar * title = get_search_value (node,TITLE_BEGIN,TITLE_ENDIN);

                    if (title != NULL)
                    {
                        gchar * delim = g_strrstr (title," chords");
                        if (delim == NULL)
                        {
                            delim = g_strrstr (title," tab");
                        }

                        if (delim != NULL)
                        {
                            delim[0] = 0;
                        }
                    }

                    /* Check if this is the item we actually search */
                    if (levenshtein_strnormcmp (capo->s,title, capo->s->title ) <= capo->s->fuzzyness &&
                            levenshtein_strnormcmp (capo->s,artist,capo->s->artist) <= capo->s->fuzzyness)
                    {
                        /* Build resulting url */
                        gchar * result_url = g_strdup_printf ("%s%s",GT_BASE,url);

                        /* Go and parse it */
                        GlyrMemCache * result = parse_single_page (capo->s,result_url);
                        if (result != NULL)
                        {
                            result_list = g_list_prepend (result_list,result);
                        }
                        g_free (result_url);
                    }
                    g_free (url);
                    g_free (title);

                }
                g_free (artist);
            }
        }
    }
    return result_list;
}

/////////////////////////////////

MetaDataSource guitartabs_guitaretab_src =
{
    .name = "guitaretab",
    .key  = 'g',
    .parser    = gt_guitaretabs_parse,
    .get_url   = gt_guitaretabs_url,
    .type      = GLYR_GET_GUITARTABS,
    .quality   = 95,
    .speed     = 75,
    .endmarker = NULL,
    .free_url  = false
};
