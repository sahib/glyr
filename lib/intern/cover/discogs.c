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

#define RELEASE_ID  "<release id=\""
#define RELEASE_END "\" "
#define TITLE_BEGIN "<title>"
#define TITLE_ENDIN "</title>"
#define IMGNODE_BEGIN "<image "
#define URL_BEGIN "uri=\""
#define IMG_TYPE "primary"
#define NODE_END "\" "

static const gchar * cover_discogs_url(GlyrQuery * sets)
{
	if(sets->img_max_size >= 300 || sets->img_max_size == -1)
	{
		return "http://www.discogs.com/artist/${artist}?f=xml&api_key="API_KEY_DISCOGS;
	}
	return NULL;
}

/*------------------------------------------------*/

static gboolean check_image_size(GlyrQuery * query, gchar * image_begin)
{
	gboolean result = FALSE;
	gchar * height = get_search_value(image_begin,"height=\"",NODE_END);
	gchar * width  = get_search_value(image_begin,"width=\"", NODE_END);
	if(width && height)
	{
		gint numeric_width  = strtol(width, NULL,10);
		gint numeric_height = strtol(height,NULL,10);
		gint ratio = (numeric_width + numeric_height) / 2;
		result = size_is_okay(ratio,query->img_min_size,query->img_max_size);

		g_free(height);
		g_free(width);
	}
	return result;
}

/*------------------------------------------------*/

static void parse_single_page(GlyrQuery * query, GlyrMemCache * tmp_cache, GList ** result_list)
{
	if(tmp_cache != NULL)
	{
		/* Parsing the image url from here on */
		char * imgurl_begin = tmp_cache->data;
		while((imgurl_begin = strstr(imgurl_begin + (sizeof IMGNODE_BEGIN) - 1,IMGNODE_BEGIN)) != NULL)
		{
			GLYR_DATA_TYPE img_type;
			gchar * is_primary = strstr(imgurl_begin,IMG_TYPE);
			if(((is_primary - imgurl_begin) > 42))
				continue;

			if(is_primary != NULL)
				img_type = GLYR_TYPE_COVERART_PRI;
			else
				img_type = GLYR_TYPE_COVERART_SEC;


			if(check_image_size(query,imgurl_begin) == TRUE)
			{
				gchar * final_url = get_search_value(imgurl_begin,URL_BEGIN,NODE_END);
				if(final_url != NULL)
				{
					GlyrMemCache * result = DL_init();
					result->data = final_url;
					result->size = strlen(result->data);
					result->type = img_type;
					result->dsrc = g_strdup(tmp_cache->dsrc);
					*result_list = g_list_prepend(*result_list,result);
				}
			}
		}
		DL_free(tmp_cache);
	}
}

/*------------------------------------------------*/

static gboolean validate_title(GlyrQuery * query, gchar * release_node)
{
	gboolean result = FALSE;
	gchar * album_value = get_search_value(release_node,TITLE_BEGIN,TITLE_ENDIN);
	if(album_value && levenshtein_strnormcmp(query,album_value,query->album) <= query->fuzzyness)
	{
		result = TRUE;
	}
	g_free(album_value);
	return result;
}

/*------------------------------------------------*/

/* Most of the time there are no primary covers, just to be sure */
static void sort_primary_before_secondary(GList ** result_list)
{
	if(result_list != NULL && *result_list != NULL)
	{
		GList * last_element = *result_list;
		while(TRUE)
		{
			GList * elem = NULL;
			for(elem = last_element; elem; elem = elem->next)
			{
				GlyrMemCache * item = elem->data;
				if(item->type == GLYR_TYPE_COVERART_PRI)
				{
					last_element = elem->next;
					*result_list = g_list_delete_link(*result_list,elem);
					*result_list = g_list_prepend(*result_list,item);
					break;
				}
			}

			if(elem == NULL) break;
		}
	}
}

/*------------------------------------------------*/

/* The discogs.com parser is a little more complicated... */
static GList * cover_discogs_parse(cb_object * capo)
{
	GList * result_list = NULL;

	/* Go through all release nodes */
	gchar * release_node = capo->cache->data;
	while(continue_search(g_list_length(result_list),capo->s) && (release_node = strstr(release_node+1,RELEASE_ID)) != NULL)
	{
		if(validate_title(capo->s,release_node) == TRUE)
		{
			/* Get release ID */
			gchar * release_end = strstr(release_node, RELEASE_END);
			if(release_end != NULL)
			{
				// Copy ID from cache
				gchar * release_ID = copy_value(release_node + (sizeof RELEASE_ID) - 1,release_end);
				if(release_ID != NULL)
				{
					/*Construct release_url */
					gchar *release_url = g_strdup_printf("http://www.discogs.com/release/%s?f=xml&api_key="API_KEY_DISCOGS,release_ID);
					if(release_url != NULL)
					{
						/* Only download till artists tag */
						GlyrMemCache * tmp_cache = download_single(release_url,capo->s,"<artists>");
						parse_single_page(capo->s,tmp_cache,&result_list);
						g_free(release_url);
					}
					g_free(release_ID);
				}
			}
		}
	}
	sort_primary_before_secondary(&result_list);
	return result_list;
}

/*------------------------------------------------*/

MetaDataSource cover_discogs_src =
{
	.name      = "discogs",
	.key       = 'd',
	.parser    = cover_discogs_parse,
	.get_url   = cover_discogs_url,
	.type      = GLYR_GET_COVERART,
	.quality   = 50,
	.speed     = 35,
	.endmarker = NULL,
	.free_url  = false
};
