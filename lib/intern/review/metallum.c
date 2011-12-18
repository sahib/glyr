/***********************************************************
 * This file is part of glyr
 * + a commnandline tool and library to download various sort of musicrelated metadata.
 * + Copyright (C) [2011-2012]  [Christopher Pahl]
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

/* Monstress of URL - thanks Wireshark.. */
#define AJAX_URL "http://www.metal-archives.com/search/ajax-advanced/searching/albums/" \
 	         "?bandName=${artist}&exactBandMatch=1&releaseTitle=${album}"           \
	         "&exactReleaseMatch=0&releaseType=&_=1313619222250&sEcho=1"            \
		 "&iColumns=3&sColumns=&iDisplayStart=0"                                \
		 "&iDisplayLength=100&sNames=%%2C%%2C"

static const gchar * review_metallum_url(GlyrQuery * s)
{
    return AJAX_URL;
}

#define REVIEW_START "<div class=\"reviewContent\">"
#define REVIEW_END   "</div>"

static void parse_review_site(GlyrQuery * s, GlyrMemCache * cache, GList ** result_items)
{
	if(cache != NULL)
	{
		gsize nodelen = (sizeof REVIEW_START) - 1;
		gchar * node  = cache->data;

		while(continue_search(g_list_length(*result_items),s) && (node = strstr(node+nodelen,REVIEW_START)) != NULL)
		{
			gchar * data = get_search_value(node,REVIEW_START,REVIEW_END);
			if(data != NULL)
			{
				GlyrMemCache * item = DL_init();

				gchar * kill_br = strreplace(data,"  <br />\n","");
				item->data = strreplace(kill_br,".  ",".\n");
				item->size = strlen(item->data);
				item->dsrc = g_strdup(cache->dsrc);
				*result_items = g_list_prepend(*result_items, item);

				g_free(kill_br);
				g_free(data);
			}
		}
		DL_free(cache);
	}
}

#define NODE_START "\"<a href=\\\""
#define NODE_END "\\\">"

static GList * review_metallum_parse(cb_object * capo)
{
	GList * result_items = NULL;

	gsize nodelen = strlen(NODE_START);
	gchar * node  = capo->cache->data;
	gint node_ctr = 0;

	while((node = strstr(node+nodelen,NODE_START)) != NULL)
	{
		/* Only take the album url, not the other urls */
		if(++node_ctr % 2 == 0)
		{
			node += nodelen;
			gchar * content_url = copy_value(node,strstr(node,NODE_END));
			if(content_url != NULL)
			{
				gchar * review_url = strreplace(content_url,"/albums/","/reviews/");
				if(review_url != NULL)
				{
					parse_review_site(capo->s, download_single(review_url,capo->s,NULL), &result_items);
					g_free(review_url);
				}
				g_free(content_url);
			}
		}
	}
	return result_items;
}


MetaDataSource review_metallum_src =
{
	.name = "metallum",
	.key  = 'e',
	.parser    = review_metallum_parse,
	.get_url   = review_metallum_url,
	.endmarker = NULL,
	.quality   = 95,
	.speed     = 70,
	.free_url  = false,
	.type      = GLYR_GET_ALBUM_REVIEW
};

