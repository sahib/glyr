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

#include "../../stringlib.h"
#include "../../core.h"

const char * cover_lyricswiki_url(GlyrQuery * sets)
{
    if(sets->img_min_size < 500)
        return "http://lyrics.wikia.com/api.php?format=xml&action=query&list=allimages&aiprefix=%artist%";

    return NULL;
}


/**
-- Example snippet
<img name="Axxis_-_Access_All_Areas.jpg"
	timestamp="2010-08-03T17:05:20Z"
	url="http://images.wikia.com/lyricwiki/images/f/f9/Axxis_-_Access_All_Areas.jpg"
	descriptionurl="http://lyrics.wikia.com/File:Axxis_-_Access_All_Areas.jpg"
/>
**/

#define IMG_TAG "_-_"
#define END_TAG "\" timestamp=\""
#define URL_MARKER "url=\""
#define URL_END "\" descriptionurl="

GList * cover_lyricswiki_parse(cb_object * capo)
{
    char * find=capo->cache->data;
    char * endTag   = NULL;

    int urlc = 0;
    GList * r_list = NULL;

    char *tmp = strreplace(capo->s->album," ","_");
    if(tmp)
    {
        char * _album = g_utf8_strdown(tmp,-1);
        if(_album)
        {
            // Go through all names and compare them with Levenshtein
            while(find && (find = strstr(find+1,IMG_TAG)) != NULL && continue_search(urlc,capo->s))
            {
                // Find end & start of the name
                find  += strlen(IMG_TAG);
                endTag = strstr(find,END_TAG);

                if(endTag == NULL || endTag <= find)
                    continue;

                // Copy the name of the current album
                char * name = copy_value(find,endTag);
                if(name)
                {
                    // Remove .png / .jpg ...
                    int ll = endTag - find;
                    for(; ll; ll--)
                        if(name[ll] == '.')
                            name[ll] = '\0';

                    // Compare only lower case strings...
                    gchar * down_name = g_utf8_strdown(name,-1);
                    if(down_name)
                    {
                        // Allow max. 2 'typos'
                        if(levenshtein_strcmp(_album, down_name) < capo->s->fuzzyness)
                        {
                            char *url_start = strstr(endTag,URL_MARKER);
                            if(url_start)
                            {
                                url_start += strlen(URL_MARKER);
                                char *url_end   = strstr(url_start,URL_END);
                                if(url_end)
                                {
                                    char * url = copy_value(url_start, url_end);
                                    if(url)
                                    {
                                        GlyrMemCache * result = DL_init();
                                        result->data = url;
                                        result->size = url_end - url_start;
                                        r_list = g_list_prepend(r_list,result);
                                        urlc++;
                                    }

                                    url_end=NULL;
                                }

                                url_start=NULL;
                            }
                        }

                        // Get next img tag
                        find = strstr(endTag,"<img name=\"");
                        g_free(down_name);
                        down_name=NULL;
                    }
                }
                g_free(name);
                name=NULL;
            }
            g_free(_album);
            _album=NULL;
        }

        g_free(tmp);
        tmp=NULL;
    }
    return r_list;
}

/*-----------------------------------------------*/

MetaDataSource cover_lyricswiki_src =
{
    .name      = "lyricswiki",
    .key       = 'w',
    .parser    = cover_lyricswiki_parse,
    .get_url   = cover_lyricswiki_url,
    .type      = GET_COVERART,
    .quality   = 75,
    .speed     = 65,
    .endmarker = NULL,
    .free_url  = false
};
