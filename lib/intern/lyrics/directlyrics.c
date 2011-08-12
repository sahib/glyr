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

// directlyrics only offers google/javascipt powered search
// Unable to use this via CGI without lots of pain..
// So only use dump URL scheme

const char * lyrics_directlyrics_url(GlyrQuery * settings)
{
    gchar * url   = NULL;
    gchar * esc_a = g_utf8_strdown(settings->artist,-1);
    if(esc_a != NULL)
    {
        gchar * esc_t = g_utf8_strdown(settings->title,-1);
        if(esc_t != NULL)
        {
            gchar * rep_a = strreplace(esc_a," ","-");
            if(rep_a != NULL)
            {
                gchar * rep_t = strreplace(esc_t," ","-");
                if(rep_t != NULL)
                {
                    url =  g_strdup_printf("http://www.directlyrics.com/%s-%s-lyrics.html",rep_a,rep_t);
                    g_free(rep_t);
                }
                g_free(rep_a);
            }
            g_free(esc_t);
        }
        g_free(esc_a);
    }
    return url;
}

#define START "<div id=\"lyricsContent\"><p>"
#define ENDIN "</div>"

GList * lyrics_directlyrics_parse(cb_object * capo)
{
    char * f_entry;
    GlyrMemCache * result = NULL;
    GList * r_list = NULL;

    if( (f_entry = strstr(capo->cache->data,START)) )
    {
        char * f_end = NULL;
        f_entry += strlen(START);
        if( (f_end = strstr(f_entry,ENDIN)) )
        {
            char * buf = copy_value(f_entry,f_end);
            if(buf)
            {
                // replace nonsense brs that glyr would expand to newlines
                char * brtagged = strreplace(buf,"<br>","");
                if(brtagged)
                {
                    result = DL_init();
                    result->data = brtagged;
                    result->size = strlen(brtagged);
                    result->dsrc = strdup(capo->url);
                }
                g_free(buf);
            }
        }
    }

    if(result)
    {
        r_list = g_list_prepend(r_list,result);
    }
    return r_list;
}

/*---------------------------------------------------*/

MetaDataSource lyrics_directlyrics_src =
{
    .name = "directlyrics",
    .key  = 'i',
    .parser    = lyrics_directlyrics_parse,
    .get_url   = lyrics_directlyrics_url,
    .type      = GET_LYRICS,
    .endmarker = "<!-- google_ad_section_start(weight=ignore) -->",
    .quality   = 30,
    .speed 	   = 80,
    .free_url  = true
};
