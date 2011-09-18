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

#define GLYR_GET_URL "http://www.lyricstime.com/search/?q=${artist}+${title}&t=default"

/*--------------------------------------------------------*/

static const char * lyrics_lyricstime_url(GlyrQuery * settings)
{
    return GLYR_GET_URL;
}

/*--------------------------------------------------------*/

#define LYR_BEGIN "<div id=\"songlyrics\" >"
#define LYR_ENDIN "</div>"

static GlyrMemCache * parse_page(GlyrMemCache * dl, cb_object * capo)
{
	GlyrMemCache * result = NULL;
	if(dl != NULL)
	{
		gchar * begin = strstr(dl->data,LYR_BEGIN);
		if(begin != NULL)
		{
			begin += (sizeof LYR_BEGIN) - 1;
			gchar * end = strstr(begin,LYR_ENDIN);
			if(end != NULL)
			{
				*(end) = 0;
				gchar * no_br_tags = strreplace(begin,"<br />",NULL);
				if(no_br_tags != NULL)
				{
					result = DL_init();
					result->data = beautify_string(no_br_tags);
					result->size = (result->data) ? strlen(result->data) : 0;
					result->dsrc = g_strdup(dl->dsrc);

					g_free(no_br_tags);
				}
			}
		}
	}
	return result;
}

/*--------------------------------------------------------*/

#define START_SEARCH "<div id=\"searchresult\">"
#define SEARCH_ENDIN "</div>"
#define NODE_BEGIN   "<li><a href=\""
#define NODE_ENDIN   "\">"
#define SPAN_BEGIN "<span class"
#define ARTIST_BEG "<b>"
#define ARTIST_END "</b>"

static gboolean validate_artist(cb_object * capo, gchar * backpointer)
{
	gboolean i_shall_continue = false;
	if(backpointer != NULL)
	{
		char * span = strstr(backpointer,SPAN_BEGIN);
		if(span != NULL)
		{
			gchar * artist_beg = strstr(span,ARTIST_BEG);
			if(artist_beg != NULL)
			{
				artist_beg += (sizeof ARTIST_BEG) - 1;
				gchar * artist_end = strstr(artist_beg,ARTIST_END);
				if(artist_end != NULL)
				{
					gchar * artist_val = copy_value(artist_beg,artist_end);
					if(artist_val != NULL)
					{
						if(levenshtein_strnormcmp(capo->s,artist_val,capo->s->artist) <= capo->s->fuzzyness)
						{
							i_shall_continue = true;
						}
						g_free(artist_val);
					}
				}
			}
		}
	}
	return i_shall_continue;
}

/*--------------------------------------------------------*/

static GList * lyrics_lyricstime_parse(cb_object * capo)
{
	GList * rList = NULL;
	char * start = capo->cache->data;
	if(start != NULL)
	{
		gchar * div_end = strstr(start,SEARCH_ENDIN);
		gchar * node = capo->cache->data;
		gchar * backpointer = node;
		gsize nlen = (sizeof NODE_BEGIN) - 1;

		while(continue_search(g_list_length(rList),capo->s) && (node = strstr(node+nlen,NODE_BEGIN)) != NULL)
		{
			if(div_end >= node)
				break;

			if(validate_artist(capo,backpointer) == TRUE)
			{
				gchar * end_of_url = strstr(node+nlen,NODE_ENDIN);
				if(end_of_url != NULL)
				{
					gchar * url = copy_value(node+nlen,end_of_url);
					if(url != NULL)
					{
						gchar * full_url = g_strdup_printf("http://www.lyricstime.com%s",url);
						GlyrMemCache * dl_cache = download_single(full_url,capo->s,NULL);
						if(dl_cache)
						{
							GlyrMemCache * parsed_cache = parse_page(dl_cache,capo);
							if(parsed_cache != NULL)
							{
								rList = g_list_prepend(rList,parsed_cache);
							}
							DL_free(dl_cache);
							g_free(full_url);
						}
						g_free(url);
					}
				}
			}
			backpointer = node;
		}
	}
	return rList;
}

/*--------------------------------------------------------*/

MetaDataSource lyrics_lyricstime_src =
{
	.name = "lyricstime",
	.key  = 't',
	.parser    = lyrics_lyricstime_parse,
	.get_url   = lyrics_lyricstime_url,
	.type      = GLYR_GET_LYRICS,
	.quality   = 70,
	.speed     = 60,
	.endmarker = NULL,
	.free_url  = false
};
