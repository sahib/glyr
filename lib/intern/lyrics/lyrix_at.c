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

#define AT_URL "http://lyrix.at/lyrics-search/s-%artist%,,%title%,,any,1321,0.html"

const char * lyrics_lyrixat_url(GlyrQuery * settings)
{
    return AT_URL;
}

#define SEARCH_START_TAG "<!-- start of result item //-->"
#define LYRIC_BEGIN "<div class='songtext' id='stextDIV'>"
#define URL_TAG_BEGIN "<a href='/de"
#define URL_TAG_ENDIN "'>"

#define MAX_TRIES 5

GList * lyrics_lyrixat_parse(cb_object * capo)
{
    /* lyrix.at does not offer any webservice -> use the searchfield to get some results */
    GList * r_list = NULL;
    char * search_begin_tag = capo->cache->data;
    int ctr = 0, urlc = 0;

    while( (search_begin_tag = strstr(search_begin_tag+1,SEARCH_START_TAG)) && MAX_TRIES >= ctr++ && continue_search(urlc,capo->s))
    {
        char * url_tag = search_begin_tag;
        size_t toggle = 1,i;

        for(i=0; i < toggle && url_tag; i++)
            url_tag = strstr(url_tag,URL_TAG_BEGIN);

        if(url_tag)
        {
            char * title_tag = strstr(url_tag,URL_TAG_ENDIN);
            if(title_tag)
            {
                char * title_end = strstr(title_tag,"<");
                if(title_end)
                {
                    char * title = copy_value(title_tag + strlen(URL_TAG_ENDIN),title_end);
                    if(title)
                    {
                        if(levenshtein_strcmp(title,capo->s->title) <= capo->s->fuzzyness)
                        {
                            char * url_part = copy_value(url_tag+strlen(URL_TAG_BEGIN),title_tag);
                            if(url_part)
                            {
                                char * url = g_strdup_printf("http://lyrix.at/de%s",url_part);
                                if(url)
                                {
                                    GlyrMemCache * lyrcache = download_single(url,capo->s,"<!-- eBay Relevance Ad -->");
                                    if(lyrcache)
                                    {
                                        char * lyr_begin = strstr(lyrcache->data,LYRIC_BEGIN);
                                        if(lyr_begin)
                                        {
                                            char * lyr_endin = strstr(lyr_begin,"<div>");
                                            if(lyr_endin)
                                            {
                                                char * lyrics = copy_value(lyr_begin,lyr_endin);
                                                if(lyrics)
                                                {
                                                    GlyrMemCache * result = DL_init();
                                                    result->data = strreplace(lyrics,"<br />","");
                                                    result->size = strlen(lyrics);
                                                    result->dsrc = strdup(url);
                                                    r_list = g_list_prepend(r_list,result);
                                                    urlc++;
                                                }
                                                g_free(lyrics);
                                            }
                                            lyr_begin=NULL;
                                        }
                                        DL_free(lyrcache);
                                    }
                                    g_free(url);
                                }
                                g_free(url_part);
                            }
                        }
                        g_free(title);
                    }
                    title_end=NULL;
                }
            }
        }
    }
    return r_list;
}
/*--------------------------------------------------------*/

MetaDataSource lyrics_lyrix_src =
{
    .name = "lyrix",
    .key  = 'a',
    .parser    = lyrics_lyrixat_parse,
    .get_url   = lyrics_lyrixat_url,
    .type      = GET_LYRICS,
    .endmarker = "<div id='d_navigation'",
    .quality   = 70,
    .speed     = 50,
    .free_url  = false
};
