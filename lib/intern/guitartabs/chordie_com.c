/***********************************************************
* This file is part of glyr
* + a commnadline tool and library to download various sort of music related metadata.
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

#define BASE_URL "http://www.chordie.com"
#define SEARCH_URL BASE_URL"/?q=${artist}%20${title}&np=0&ps=10&wf=2221&s=RPD&wf=2221&wm=wrd&type=&sp=1&sy=1&cat=&ul=&np=0"

#define RESULTS_BEGIN "<!--results start-->"
#define RESULTS_ENDIN "<!--results end-->"
#define NODE "<a class=\"darkblue\" href=\""

static const gchar * guitartabs_chordie_url (GlyrQuery * s)
{
    return SEARCH_URL;
}

/////////////////////////////////

static gboolean check_title_value (GlyrQuery * s, const gchar * to_check)
{
    gboolean result = FALSE;
    if (to_check != NULL)
    {
        gchar * artist = g_strrstr (to_check,"(");
        if (artist != NULL)
        {
            artist[0] = 0;
            artist++;

            gchar * end_bracket = strchr (artist,')');
            if (end_bracket)
            {
                end_bracket[0] = 0;
            }

            result = (levenshtein_strnormcmp (s,artist,s->artist)  <= s->fuzzyness &&
                      levenshtein_strnormcmp (s,to_check,s->title) <= s->fuzzyness);
        }
    }
    return result;
}

/////////////////////////////////

static GlyrMemCache * parse_result_page (GlyrQuery * s, gchar * content_url)
{
    GlyrMemCache * result = NULL;
    if (content_url != NULL)
    {
        GlyrMemCache * dl_cache = download_single (content_url,s,NULL);
        if (dl_cache != NULL)
        {
            gchar * content = get_search_value (dl_cache->data,"<div class=\"song\">","</div>");
            if (content != NULL)
            {
                result = DL_init();
                result->data = content;
                result->size = strlen (content);
                result->dsrc = g_strdup (content_url);
            }
            DL_free (dl_cache);
        }
    }
    return result;
}


/////////////////////////////////

static GList * guitartabs_chordie_parse (cb_object * capo)
{
    GList * result_list = NULL;
    gchar * search_begin = strstr (capo->cache->data,RESULTS_BEGIN);
    if (search_begin != NULL)
    {
        gchar * search_ending = strstr (search_begin,RESULTS_ENDIN);
        if (search_ending != NULL)
        {
            gchar * node  = search_begin;
            gsize nodelen = (sizeof NODE) - 1;
            while (continue_search (g_list_length (result_list),capo->s) && (node = strstr (node + nodelen, NODE) ) != NULL && node >= search_begin && node <= search_ending)
            {
                gchar * url = get_search_value (node,NODE,"\" ");
                if (url != NULL)
                {
                    gchar * name_value = get_search_value (node,"\">","</a>");
                    if (check_title_value (capo->s, name_value) == TRUE)
                    {
                        gchar * content_url = g_strdup_printf ("%s%s",BASE_URL,url);
                        GlyrMemCache * result = parse_result_page (capo->s,content_url);
                        if (result != NULL)
                        {
                            result_list = g_list_prepend (result_list,result);
                        }
                        g_free (content_url);
                    }
                    g_free (name_value);
                    g_free (url);
                }
            }
        }
    }
    return result_list;
}

/////////////////////////////////

MetaDataSource guitartabs_chordie_src =
{
    .name = "chordie",
    .key  = 'c',
    .parser    = guitartabs_chordie_parse,
    .get_url   = guitartabs_chordie_url,
    .type      = GLYR_GET_GUITARTABS,
    .quality   = 95,
    .speed     = 75,
    .endmarker = NULL,
    .free_url  = false
};
