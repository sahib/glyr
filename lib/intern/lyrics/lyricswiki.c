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

const gchar * lyrics_lyricswiki_url(GlyrQuery * settings)
{
    return LW_URL;
}

/*--------------------------------------------------------*/

// Compare response, so lyricswiki's search did not fool us
gboolean lv_cmp_content(const gchar * to_artist, const gchar * to_title, cb_object * capo)
{
    gboolean res = false;
    if(to_artist && to_title && capo)
    {
        gchar * tmp_artist = copy_value(to_artist,strstr(to_artist,"</artist>"));
        if(tmp_artist != NULL)
        {
            gchar * tmp_title = copy_value(to_title, strstr(to_title ,"</song>" ));
            if(tmp_title != NULL)
            {
				 /* levenshtein_strnormcmp takes care of those brackets */
                 if((levenshtein_strnormcmp(capo->s->artist,tmp_artist) <= capo->s->fuzzyness &&
                     levenshtein_strnormcmp(capo->s->title, tmp_title)  <= capo->s->fuzzyness ))
                 {
                     res = true;
                 }
                 g_free(tmp_title);
            }
            g_free(tmp_artist);
        }
    }
    return res;
}


/*--------------------------------------------------------*/

#define LYR_BEGIN "'17'/></a></div>"
#define LYR_ENDIN "<!--"

GList * lyrics_lyricswiki_parse(cb_object * capo)
{
    GList * result_list = NULL;
    if(lv_cmp_content(strstr(capo->cache->data,"<artist>"),strstr(capo->cache->data,"<song>"),capo))
    {
        gchar *find, *endTag;
        if((find = strstr(capo->cache->data,"<url>")) != NULL)
        {
            nextTag(find);
            if((endTag = strstr(find, "</url>")) != NULL)
            {
                gchar * wiki_page_url = copy_value(find,endTag);
                if(wiki_page_url != NULL)
                {
                    GlyrMemCache * new_cache = download_single(wiki_page_url, capo->s,NULL);
                    if(new_cache != NULL)
                    {
                        gchar *lyr_begin, *lyr_end;
                        if( (lyr_begin = strstr(new_cache->data,LYR_BEGIN)) != NULL)
                        {
                            nextTag(lyr_begin);
                            nextTag(lyr_begin);
                            nextTag(lyr_begin);

                            if((lyr_end = strstr(lyr_begin,LYR_ENDIN)) != NULL) 
                            {
                                gchar * lyr = copy_value(lyr_begin,lyr_end);
                                if(lyr != NULL && strstr(lyr,BAD_STRING) == NULL)
                                {
                                    GlyrMemCache * result = DL_init();
                                    result->data = lyr;
                                    result->size = ABS(lyr_end - lyr_begin);
                                    result->dsrc = strdup(wiki_page_url);
        							result_list = g_list_prepend(result_list,result);
                                }
                            }
                        }
                        DL_free(new_cache);
                    }
                    g_free(wiki_page_url);
                }
            }
        }
    }
    return result_list;
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
