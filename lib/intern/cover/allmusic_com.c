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
#include "../../glyr.h"

static const char * cover_allmusic_url(GlyrQuery * s)
{
	if(s->img_min_size < 200)
	{
		return "http://www.allmusic.com/search/album/${album}";
	}
	return NULL;
}

// begin of search results
#define SEARCH_TREE_BEGIN "<table class=\"search-results\""

//*sole search result */
#define SEARCH_NODE "<td><a href=\""
#define SEARCH_DELM "\">"

/* artist */
#define ARTIST_PART "<td>"
#define ARTIST_END  "</td>"
#define IMG_BEGIN "<div class=\"image\"> <img src=\""
#define IMG_ENDIN "\" alt=\""

#define BAD_URL "/img/pages/site/icons/no_cover_200.gif"

static GlyrMemCache * parse_cover_page(GlyrMemCache * dl_cache)
{
	GlyrMemCache * result = NULL;
	if(dl_cache != NULL)
	{
		gchar * img_url = get_search_value(dl_cache->data,IMG_BEGIN,IMG_ENDIN);
		if(img_url != NULL)
		{
			if(strcmp(img_url,BAD_URL) != 0)
			{
				result = DL_init();
				result->data = img_url;
				result->size = strlen(img_url);
			}
			else
			{
				g_free(img_url);
			}
		}
	}
	return result;
}

/*--------------------------------------*/

#define TITLE_TAG "<a href=\"\">Title</a></th>"
static GList * cover_allmusic_parse(cb_object * capo)
{
	GList * result_list = NULL;
	if(strstr(capo->cache->data,TITLE_TAG) != NULL)
	{
		/* Horray, directly hit the page */
		GlyrMemCache * result = parse_cover_page(capo->cache);
		result_list = g_list_prepend(result_list, result);
		return result_list;
	}

	gchar * search_begin = NULL;
	if((search_begin = strstr(capo->cache->data, SEARCH_TREE_BEGIN)) == NULL)
	{
		/* No page. Crap. */
		return NULL;
	}

	gsize nodelen = (sizeof SEARCH_NODE) - 1;
	gchar *  node = search_begin;
	while(continue_search(g_list_length(result_list),capo->s) && (node = strstr(node + nodelen,SEARCH_NODE)))
	{
		gchar * url = get_search_value(node,SEARCH_NODE,SEARCH_DELM);
		if(url != NULL)
		{
			/* We have the URL - now check the artist to be the one */
			gchar * artist = get_search_value(node + nodelen,ARTIST_PART, ARTIST_END); 
			if(artist != NULL)
			{
				if(levenshtein_strnormcmp(capo->s,capo->s->artist,artist) <= capo->s->fuzzyness)
				{
					GlyrMemCache * dl_cache = download_single(url,capo->s,"<div class=\"artist\">");
					if(dl_cache != NULL)
					{
						GlyrMemCache * result = parse_cover_page(dl_cache);
						if(result != NULL && result->data)
						{
							result->dsrc = g_strdup(url);
							result_list = g_list_prepend(result_list,result);
						}
						DL_free(dl_cache);
					}
				}
				g_free(artist);
			}
			g_free(url);
		}
	}
	return result_list;
}

/*--------------------------------------*/

MetaDataSource cover_allmusic_src =
{
	.name      = "allmusic",
	.key       = 'm',
	.parser    = cover_allmusic_parse,
	.get_url   = cover_allmusic_url,
	.type      = GLYR_GET_COVERART,
	.quality   = 65,
	.speed     = 50,
	.endmarker = NULL,
	.free_url  = false
};
