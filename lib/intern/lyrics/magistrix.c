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

static const char * lyrics_magistrix_url(GlyrQuery * settings)
{
    return MG_URL;
}

/*--------------------------------------------------------*/

#define OPT_START "<img align=\"absmiddle\" alt=\"Phone2\""
#define OPT_END   "<img align=\"absmiddle\" alt=\"Phone\""

/*--------------------------------------------------------*/

static GlyrMemCache * parse_lyric_page(const char * buffer)
{
    GlyrMemCache * result = NULL;
    g_return_val_if_fail(buffer,NULL);

    const char * share_tag = "<ul id='share_buttons'>";

    gchar * beg_lyrics = strstr(buffer,share_tag);
    if(beg_lyrics != NULL)
    {
        gchar * end_lyrics = g_strstr_len(beg_lyrics + 1,2000,share_tag);
        if(end_lyrics != NULL)
        {
            gsize lyrics_len = (end_lyrics - beg_lyrics);
            if(lyrics_len > 20)
            {
                gchar * raw_lyrics = copy_value(beg_lyrics,end_lyrics);

                if(raw_lyrics != NULL)
                {
                    result = DL_init();

                    gchar * no_twitter = strreplace(raw_lyrics,"<a class='twitter-share-button' data-lang='de'>Twittern</a>","");
                    
                    result->data = strreplace(no_twitter,"<br />","");
                    result->size = strlen(result->data); 

                    g_free(raw_lyrics);
                    g_free(no_twitter);
                }
            }
        }
    }


    return result;
}

/*--------------------------------------------------------*/

#define ARTIST_BEGIN "class=\"artistIcon bgMove\">"
#define TITLE_BEGIN "\" class=\"lyricIcon bgMove\">"
#define ARTIST_ENDIN "</a>"
#define TITLE_ENDING "</a>"

static gboolean convert_levenshtein(GlyrQuery * s, gchar * content, gchar * compare)
{
    gboolean result = FALSE;
    if(compare != NULL)
    {
        if(levenshtein_strnormcmp(s,compare,content) <= s->fuzzyness)
        {
            result = TRUE;
        }
    }
    return result;
}

/*--------------------------------------------------------*/

static gboolean approve_content(GlyrQuery * s, gchar * node)
{
    gboolean result = FALSE;
    gchar * artist_begin = strstr(node,ARTIST_BEGIN);
    if(artist_begin != NULL)
    {
        gsize artist_beg_len = (sizeof ARTIST_BEGIN) - 1;
        gchar * artist = copy_value(artist_begin + artist_beg_len, strstr(node,ARTIST_ENDIN));
        if(artist != NULL)
        {
            if(convert_levenshtein(s,artist,s->artist))
            {
                gchar * title_begin = strstr(node,TITLE_BEGIN);
                if(title_begin != NULL)
                {
                    gsize title_beg_len = (sizeof TITLE_BEGIN) - 1;
                    gchar * title = copy_value(title_begin + title_beg_len,strstr(title_begin,TITLE_ENDING));
                    if(title != NULL)
                    {
                        if(convert_levenshtein(s,title,s->title))
                        {
                            result = TRUE;
                        }
                        g_free(title);
                    }
                }
            }
            g_free(artist);
        }
    }
    return result;
}

/*--------------------------------------------------------*/

#define URL_BEGIN "<a href=\""
#define MAX_TRIES 5
#define NODE_BEGIN "<tr class='topLine'>"
#define COMMENT_ENDMARK "<div class='comments'"

/*--------------------------------------------------------*/

static void query_search_page_results(cb_object * capo, GList ** result_list)
{
    gchar * node = capo->cache->data;
    gint pages_tried_counter = 0;
    gsize node_begin_len = (sizeof NODE_BEGIN) - 1;
    while(continue_search(g_list_length(*result_list),capo->s) && (node = strstr(node+node_begin_len,NODE_BEGIN)) && MAX_TRIES >= pages_tried_counter)
    {
        if(approve_content(capo->s, node) == TRUE)
        {
            gchar * url_begin = strstr(node,URL_BEGIN);
            if(url_begin != NULL)
            {
                gsize url_begin_len = (sizeof URL_BEGIN) - 1;
                url_begin = strstr(url_begin + url_begin_len, URL_BEGIN);
                if(url_begin != NULL)
                {
                    url_begin += url_begin_len;
                    gchar * url = copy_value(url_begin,strstr(url_begin,TITLE_BEGIN));
                    if(url != NULL)
                    {
                        gchar * dl_url = g_strdup_printf("www.magistrix.de%s",url);
                        if(dl_url != NULL)
                        {
                            /* Skip download of comments */
                            pages_tried_counter += 1;
                            GlyrMemCache * dl_cache = download_single(dl_url,capo->s,COMMENT_ENDMARK);
                            if(dl_cache != NULL)
                            {
                                GlyrMemCache * result = parse_lyric_page(dl_cache->data);
                                if(result != NULL)
                                {
                                    result->dsrc = g_strdup(dl_url);
                                    *result_list = g_list_prepend(*result_list,result);
                                }
                                DL_free(dl_cache);
                            }
                            g_free(dl_url);
                        }
                        g_free(url);
                    }
                }
            }
        }
    }
}

/*--------------------------------------------------------*/

static GList * lyrics_magistrix_parse (cb_object * capo)
{
    GList * result_list = NULL;
    if(strstr(capo->cache->data,"<div class='empty_collection'>") == NULL)   /* "No songtext" page? */
    {
        if(strstr(capo->cache->data,"<title>Songtext-Suche</title>") == NULL)   /* Are we not on the search result page? */
        {
            GlyrMemCache * result = parse_lyric_page(capo->cache->data);
            if(result != NULL)
            {
                result_list = g_list_prepend(result_list,result);
            }
        }
        else
        {
            query_search_page_results(capo,&result_list);
        }
    }
    return result_list;
}

/*--------------------------------------------------------*/

MetaDataSource lyrics_magistrix_src =
{
    .name = "magistrix",
    .key  = 'x',
    .parser    = lyrics_magistrix_parse,
    .get_url   = lyrics_magistrix_url,
    .type      = GLYR_GET_LYRICS,
    .quality   = 60,
    .speed     = 70,
    .endmarker = NULL,
    .free_url  = false
};
