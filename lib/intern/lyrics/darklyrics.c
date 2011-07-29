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

const char * lyrics_darklyrics_url(GlyQuery * settings)
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

    GList * r_list = NULL;
    GlyMemCache * r_cache = NULL;

    char *searchstring = strdup_printf(". %s%s",capo->s->title,"</a></h3>");
    if(searchstring)
    {
        ascii_strdown_modify(searchstring);
        char *cache_copy = strdup(capo->cache->data);
        if(cache_copy)
        {
            ascii_strdown_modify(cache_copy);
            char *head = strstr(cache_copy,searchstring);
            if(head)
            {
                char *foot  = strstr(head, "<a name=");
                if(foot)
                {
                    char * html_code = copy_value(head,foot);
                    if(html_code)
                    {
                        r_cache = DL_init();
                        r_cache->data = strreplace(html_code,"<br />","");
                        r_cache->size = strlen(r_cache->data);
                        r_cache->dsrc = strdup(capo->url);
                        free(html_code);
                    }
                }
                else
                {
                    r_cache = DL_error(NO_ENDIN_TAG);
                }
            }
            free(cache_copy);
        }
        free(searchstring);
    }
    else
    {
        r_cache = DL_error(NO_BEGIN_TAG);
    }

    if(r_cache)
    {
        r_list = g_list_prepend(r_list,r_cache);
    }
    return r_list;
}

/*---------------------------------------------------*/

MetaDataSource lyrics_darklyrics_src = {
	.name = "darklyrics",
	.key  = 'y',
	.parser    = lyrics_darklyrics_parse,
	.get_url   = lyrics_darklyrics_url,
	.type      = GET_LYRICS,
	.endmarker = "<div class=\"note\">",
	.quality   = 20,
	.speed     = 20,
	.free_url  = true
};
