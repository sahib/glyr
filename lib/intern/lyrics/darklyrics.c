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

#define DARK_URL "http://darklyrics.com/lyrics/%artist%/%album%.html#1"

const char * lyrics_darklyrics_url(GlyrQuery * settings)
{
    return DARK_URL;
}

GList * lyrics_darklyrics_parse(cb_object * capo)
{
    /* We have to search for the title in the data,
     * Therefore we convert everything to lowercase
     * to increase success chances.
     * Afterwards we copy from cache->data so we have
     * the correct case in the finaly lyrics
     * */
puts(capo->cache->data);

    GList * r_list = NULL;
    GlyrMemCache * r_cache = NULL;

    gchar *searchstring = g_strdup_printf(". %s%s",capo->s->title,"</a></h3>");
    if(searchstring != NULL)
    {
        gchar * searchstr_lower = g_utf8_strdown(searchstring,-1);
        gchar *cache_copy = g_utf8_strdown(capo->cache->data,-1);

        if(cache_copy != NULL)
        {
            gchar * head = strstr(cache_copy,searchstr_lower);
            if(head != NULL)
            {
                char *foot  = strstr(head, "<a name=");
                if(foot != NULL)
                {
                    char * html_code = copy_value(head,foot);
                    if(html_code != NULL)
                    {
                        r_cache = DL_init();
                        r_cache->data = strreplace(html_code,"<br />","");
                        r_cache->size = strlen(r_cache->data);
                        r_cache->dsrc = strdup(capo->url);
                        g_free(html_code);
                    }
                }
            }
            g_free(cache_copy);
	    g_free(searchstr_lower);
        }
        g_free(searchstring);
    }


    if(r_cache)
    {
        r_list = g_list_prepend(r_list,r_cache);
    }
    return r_list;
}

/*---------------------------------------------------*/

MetaDataSource lyrics_darklyrics_src =
{
    .name = "darklyrics",
    .key  = 'y',
    .parser    = lyrics_darklyrics_parse,
    .get_url   = lyrics_darklyrics_url,
    .type      = GET_LYRICS,
    .endmarker = "<div class=\"note\">",
    .quality   = 20,
    .speed     = 20,
    .free_url  = false
};
