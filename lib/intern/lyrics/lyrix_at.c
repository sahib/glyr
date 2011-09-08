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

#define AT_URL "http://lyrix.at/lyrics-search/s-${artist},,${title},,any,1321,0.html"

const char * lyrics_lyrixat_url(GlyrQuery * settings)
{
    return AT_URL;
}

/*--------------------------------------------------------*/

#define SEARCH_START_TAG "<!-- start of result item //-->"
#define LYRIC_BEGIN "<div class='songtext' id='stextDIV'>"
#define URL_TAG_BEGIN "<a href='/de"
#define URL_TAG_ENDIN "'>"
#define TITLE_END "<"
#define MAX_TRIES 5

/*--------------------------------------------------------*/

static void parse_lyrics_page(const gchar * url, GList ** result_list, cb_object * capo)
{
	GlyrMemCache * lyrcache = download_single(url,capo->s,"<!-- eBay Relevance Ad -->");
	if(lyrcache != NULL)
	{
		gchar * lyr_begin = strstr(lyrcache->data,LYRIC_BEGIN);
		if(lyr_begin != NULL)
		{
			gchar * lyr_endin = strstr(lyr_begin,"<div>");
			if(lyr_endin != NULL)
			{
				gchar * lyrics = copy_value(lyr_begin,lyr_endin);
				if(lyrics != NULL)
				{
					GlyrMemCache * result = DL_init();
					result->data = strreplace(lyrics,"<br />","");
					result->size = strlen(result->data);
					result->dsrc = g_strdup(url);
					*result_list = g_list_prepend(*result_list,result);
				}
				g_free(lyrics);
			}
		}
		DL_free(lyrcache);
	}
}

/*--------------------------------------------------------*/

GList * lyrics_lyrixat_parse(cb_object * capo)
{
	/* lyrix.at does not offer any webservice -> use the searchfield to get some results */
	GList * result_list = NULL;
	gchar * search_begin_tag = capo->cache->data;
	gint ctr = 0;

	while(continue_search(g_list_length(result_list),capo->s) && (search_begin_tag = strstr(search_begin_tag+1,SEARCH_START_TAG)) && MAX_TRIES >= ctr++)
	{
		gchar * url_tag = search_begin_tag;
		url_tag = strstr(url_tag,URL_TAG_BEGIN);
		if(url_tag != NULL)
		{
			gchar * title_tag = strstr(url_tag,URL_TAG_ENDIN);
			if(title_tag)
			{
				gchar * title_end = strstr(title_tag,TITLE_END);
				if(title_end != NULL)
				{
					gsize tag_end_len = (sizeof URL_TAG_ENDIN) - 1;
					gchar * title = copy_value(title_tag + tag_end_len,title_end);
					if(title != NULL)
					{
						if(levenshtein_strnormcmp(capo->s,title,capo->s->title) <= capo->s->fuzzyness)
						{
							gchar * url_part = copy_value(url_tag+strlen(URL_TAG_BEGIN),title_tag);
							if(url_part != NULL)
							{
								gchar * url = g_strdup_printf("http://lyrix.at/de%s",url_part);
								parse_lyrics_page(url,&result_list,capo);
								g_free(url);
								g_free(url_part);
							}
						}
						g_free(title);
					}
				}
			}
		}
	}
	return result_list;
}
/*--------------------------------------------------------*/

MetaDataSource lyrics_lyrix_src =
{
	.name = "lyrix",
	.key  = 'a',
	.parser    = lyrics_lyrixat_parse,
	.get_url   = lyrics_lyrixat_url,
	.type      = GLYR_GET_LYRICS,
	.quality   = 70,
	.speed     = 50,
	.free_url  = false
};
