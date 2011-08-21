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

#define BAD_STRING "Special:Random" /* This has been a running gag during developement: "I want to edit metadata!" */
#define LW_URL "http://lyrics.wikia.com/api.php?action=lyrics&fmt=xml&func=getSong&artist=%artist%&song=%title%"

/*--------------------------------------------------------*/

const char * lyrics_lyricswiki_url(GlyrQuery * settings)
{
    return LW_URL;
}

/*--------------------------------------------------------*/

// Compare response, so lyricswiki's search did not fool us
// This is to prevent completely wrong results, therfore the quite high tolerance
bool lv_cmp_content(const char *to_artist, const char * to_title, cb_object * capo)
{
    bool res = false;
    if(to_artist && to_title && capo)
    {
        gchar * tmp_artist = copy_value(to_artist,strstr(to_artist,"</artist>"));
        if(tmp_artist != NULL)
        {
            gchar * tmp_title = copy_value(to_title, strstr(to_title ,"</song>" ));
            if(tmp_title != NULL)
            {
                char * cmp_a =  g_strdup_printf("<artist>%s",capo->s->artist);
                if(cmp_a != NULL)
                {
                    char * cmp_t =  g_strdup_printf("<song>%s",capo->s->title);
                    if(cmp_t != NULL)
                    {
                        if((levenshtein_strcasecmp(cmp_a,tmp_artist) <= capo->s->fuzzyness &&
                                levenshtein_strcasecmp(cmp_t,tmp_title)  <= capo->s->fuzzyness ))
                        {
                            res = true;
                        }
                        g_free(cmp_t);
                    }
                    g_free(cmp_a);
                }
                g_free(tmp_title);
            }
            g_free(tmp_artist);
        }
    }
    return res;
}


/*--------------------------------------------------------*/

GList * lyrics_lyricswiki_parse(cb_object * capo)
{
    GlyrMemCache * result = NULL;
    GList * r_list = NULL;

    if(lv_cmp_content(strstr(capo->cache->data,"<artist>"),strstr(capo->cache->data,"<song>"),capo))
    {
        char *find, *endTag;
        if( (find = strstr(capo->cache->data,"<url>")))
        {
            nextTag(find);
            if( (endTag = strstr(find, "</url>")))
            {
                char * wiki_page_url = copy_value(find,endTag);
                if(wiki_page_url)
                {
                    GlyrMemCache * new_cache = download_single(wiki_page_url, capo->s,NULL);
                    if(new_cache)
                    {
                        char *lyr_begin, *lyr_end;
                        if( (lyr_begin = strstr(new_cache->data, "'17'/></a></div>")) )
                        {
                            nextTag(lyr_begin);
                            nextTag(lyr_begin);
                            nextTag(lyr_begin);

                            if( (lyr_end = strstr(lyr_begin, "<!--")))
                            {
                                char * lyr = copy_value(lyr_begin,lyr_end);
                                if(lyr != NULL && strstr(lyr,BAD_STRING) == NULL)
                                {
                                    result = DL_init();
                                    result->data = lyr;
                                    result->size = ABS(lyr_end - lyr_begin);
                                    result->dsrc = strdup(wiki_page_url);
                                }
                                lyr_end=NULL;
                            }
                            lyr_begin=NULL;
                        }
                        DL_free(new_cache);
                    }
                    g_free(wiki_page_url);
                }
                endTag=NULL;
            }
            find=NULL;
        }
    }

    if(result)
    {
        r_list = g_list_prepend(r_list,result);
    }
    return r_list;
}

/*--------------------------------------------------------*/

MetaDataSource lyrics_lyricswiki_src =
{
    .name = "lyricswiki",
    .key  = 'w',
    .parser    = lyrics_lyricswiki_parse,
    .get_url   = lyrics_lyricswiki_url,
    .type      = GET_LYRICS,
    .quality   = 95,
    .speed     = 95,
    .endmarker = NULL,
    .free_url  = false
};
