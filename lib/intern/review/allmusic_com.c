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

// begin of search results
#define SEARCH_TREE_BEGIN "<table class=\"search-results\""

// sole search result
#define SEARCH_NODE "<td><a href=\""
#define SEARCH_DELM "\">"

// artist
#define ARTIST_PART "<td>"
#define ARTIST_END  "</td>"

/*---------------------------------*/

static const gchar * review_allmusic_url(GlyrQuery * s)
{
    return "http://www.allmusic.com/search/album/${album}";
}

#define TEXT_BEGIN "<p class=\"text\">"
#define TEXT_ENDIN "</p>"

/*---------------------------------*/

static GlyrMemCache * parse_text(GlyrMemCache * to_parse)
{
    GlyrMemCache * return_cache = NULL;
    gchar * text = get_search_value(to_parse->data,TEXT_BEGIN,TEXT_ENDIN);

    if(text != NULL)
    {
	    return_cache = DL_init();
	    return_cache->data = text;
	    return_cache->size = strlen(text);
	    return_cache->dsrc = g_strdup(to_parse->dsrc);
    }
    return return_cache;
}

/*---------------------------------*/

#define TITLE_BEGIN  "<a href=\"\">Title</a></th>"
#define SINGLE_ENDMARK "<div id=\"tracks\">"

static GList * review_allmusic_parse(cb_object * capo)
{
	GList * result_list = NULL;
	if(strstr(capo->cache->data,TITLE_BEGIN) != NULL)
	{
		/* We're directly on the start site :-( */
		GlyrMemCache * result = parse_text(capo->cache);
		result_list = g_list_prepend(result_list, result);
		return result_list;
	}

	gchar * search_begin = NULL;
	if( (search_begin = strstr(capo->cache->data, SEARCH_TREE_BEGIN)) == NULL)
	{
		/* Oh, nothing found :-( */
		return NULL;
	}

	gsize nodelen = (sizeof SEARCH_NODE) - 1;
	gchar *  node = search_begin;
	while(continue_search(g_list_length(result_list),capo->s) && (node = strstr(node + nodelen,SEARCH_NODE)))
	{
		gchar * url = copy_value(node + nodelen,strstr(node,SEARCH_DELM));
		if(url != NULL)
		{
			/* We have the URL - now check the artist to be the one we want */
			gchar * artist = get_search_value(node + nodelen, ARTIST_PART, ARTIST_END);
			if(artist != NULL)
			{
				if(levenshtein_strnormcmp(capo->s,capo->s->artist,artist) <= capo->s->fuzzyness)
				{
					gchar * review_url = g_strdup_printf("%s/review",url);
					if(review_url != NULL)
					{
						GlyrMemCache * dl = download_single(review_url,capo->s,SINGLE_ENDMARK);
						if(dl != NULL)
						{
							GlyrMemCache * result = parse_text(dl);
							if(result != NULL)
							{
								result_list = g_list_prepend(result_list,result);
							}
							DL_free(dl);
						}
						g_free(review_url);
					}
				}
				g_free(artist);
			}
			g_free(url);
		}
	}
	return result_list;
}

/*--------------------------------------------------------*/

MetaDataSource review_allmusic_src =
{
	.name = "allmusic",
	.key  = 'm',
	.parser    = review_allmusic_parse,
	.get_url   = review_allmusic_url,
	.quality   = 75,
	.speed     = 40,
	.free_url  = false,
	.type      = GLYR_GET_ALBUM_REVIEW
};
