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
#include "../../utils.h"

#define RELEASE_ID  "<release id=\""
#define RELEASE_END "\" "
#define TITLE_BEGIN "<title>"
#define TITLE_ENDIN "</title>"
#define IMGURL_BEGIN "<image height=\""
#define IMGURL_ENDIN "\" type=\""
#define URL_BEGIN "uri=\""
#define URL_ENDIN "\" uri150="
#define IMG_TYPE "primary"

const gchar * cover_discogs_url(GlyrQuery * sets)
{
    if(sets->img_max_size >= 300 || sets->img_max_size == -1)
    {
        return "http://www.discogs.com/artist/%artist%?f=xml&api_key="API_KEY_DISCOGS;
    }
    return NULL;
}

/*------------------------------------------------*/

static gboolean check_image_size(GlyrQuery * query, gchar * image_begin, gchar * image_ending)
{
	gboolean result = FALSE;
	gsize offset = (sizeof IMGURL_BEGIN) - 1;
	gchar * size_string = copy_value(image_begin + offset, image_ending);
	if(size_string != NULL)
	{
		gsize numeric_size = strtol(size_string,NULL,10);
		result = size_is_okay(numeric_size,query->img_min_size,query->img_max_size);
		g_free(size_string);
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
				while(continue_search(g_list_length(*result_list),query) && (imgurl_begin = strstr(imgurl_begin+1,IMGURL_BEGIN)) != NULL)
				{
						gint img_type;
						gchar * is_primary = strstr(imgurl_begin,IMG_TYPE);
						if(((is_primary - imgurl_begin) > 42) )
								continue;

						if(is_primary != NULL)
								img_type = TYPE_COVER_PRI;
						else
								img_type = TYPE_COVER_SEC;

						gchar * imgurl_ending = strstr(imgurl_begin,IMGURL_ENDIN);
						if(imgurl_ending != NULL && check_image_size(query,imgurl_begin, imgurl_ending) == TRUE)
						{
								gchar * final_url_begin = strstr(imgurl_ending,URL_BEGIN);
								if(final_url_begin != NULL)
								{
										gchar * final_url = copy_value(final_url_begin + (sizeof URL_BEGIN)- 1, strstr(final_url_begin,URL_ENDIN));
										if(final_url != NULL)
										{
												GlyrMemCache * result = DL_init();
												result->data = final_url;
												result->size = strlen(result->data);
												result->type = img_type;
												*result_list = g_list_prepend(*result_list,result);
										}
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

		/* Find title in node */
		gchar * title_begin = strstr(release_node,TITLE_BEGIN);
		if(!title_begin) return result;

		/* Find end of value */
		gchar * title_endin = strstr(release_node,TITLE_ENDIN);
		if(!title_endin) return result;

		/* Go to beginning of value and get it */
		gchar * title_value = copy_value(title_begin + (sizeof TITLE_BEGIN) - 1, title_endin);
		if(levenshtein_strcasecmp(title_value,query->album) <= query->fuzzyness)
		{
				result = TRUE;
		}
		g_free(title_value);
		return result;
}

/*------------------------------------------------*/

/* The discogs.com parser is a little more complicated... */
GList * cover_discogs_parse(cb_object * capo)
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
						if(release_end)
						{
								// Copy ID from cache
								gchar * release_ID = copy_value(release_node+strlen(RELEASE_ID),release_end);
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
		return result_list;
}

/*------------------------------------------------*/

MetaDataSource cover_discogs_src =
{
		.name      = "discogs",
		.key       = 'd',
		.parser    = cover_discogs_parse,
		.get_url   = cover_discogs_url,
		.type      = GET_COVERART,
		.quality   = 50,
		.speed     = 35,
		.endmarker = NULL,
		.free_url  = false
};
