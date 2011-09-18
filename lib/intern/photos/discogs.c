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

#define PARSE_START "<artist><images>"
#define PARSE_END   "</images>"
#define NODE_START  "<image "
#define NODE_END    "\" "

static const gchar * photos_discogs_url(GlyrQuery * sets)
{
    if(sets->img_max_size >= 250 || sets->img_max_size == -1)
    {
        return "http://www.discogs.com/artist/${artist}?f=xml&api_key="API_KEY_DISCOGS;
    }
    return NULL;
}

/*------------------------------------------------*/

static gboolean check_size(GlyrQuery * s, gchar * ref)
{
	gboolean result = FALSE;
	gchar * height_string = get_search_value(ref,"height=\"",NODE_END);
	gchar * width_string  = get_search_value(ref,"width=\"",NODE_END);
	if(height_string && width_string)
	{
			gint height_num = strtol(height_string,NULL,10);
			gint width_num  = strtol(width_string, NULL,10);
			gint ratio = (height_num + width_num) / 2;

			result = size_is_okay(ratio,s->img_min_size,s->img_max_size);

			g_free(height_string);
			g_free(width_string);
	}
	return result;
}

/*------------------------------------------------*/

static GList * photos_discogs_parse(cb_object * capo)
{
	gchar * parse_start = strstr(capo->cache->data,PARSE_START);
	gchar * parse_end   = strstr(capo->cache->data,PARSE_END);

	GList * result_list = NULL;
	GHashTable * type_table = g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL,g_free);

	if(parse_start != NULL && parse_end != NULL)
	{
		gchar * node = parse_start;
		gsize node_next_len = (sizeof NODE_START) - 1;
		while((node = strstr(node + node_next_len,NODE_START)) != NULL)
		{
			node += node_next_len;
			if(check_size(capo->s,node) == TRUE)
			{
				gchar * type = get_search_value(node,"type=\"",NODE_END);
				gchar * url  = get_search_value(node,"uri=\"",NODE_END);
				if(url != NULL)
				{
					GlyrMemCache * result = DL_init();
					result->data = url;
					result->size = strlen(url);

					result_list = g_list_prepend(result_list,result);
					g_hash_table_insert(type_table,result,type);
				}
			}
		}
	}

	/* Make 'primary' the first in the list */	
	for(GList * elem = result_list; elem; elem = elem->next)
	{
		GlyrMemCache * item = elem->data;
		gchar * type_descr = g_hash_table_lookup(type_table,item);
		if(type_descr && g_ascii_strcasecmp(type_descr,"primary") == 0)
		{
			result_list = g_list_delete_link(result_list,elem);
			result_list = g_list_prepend(result_list,item);
			break;
		}
	}
	g_hash_table_destroy(type_table);
	return result_list;
}

/*------------------------------------------------*/

MetaDataSource photos_discogs_src =
{
	.name      = "discogs",
	.key       = 'd',
	.parser    = photos_discogs_parse,
	.get_url   = photos_discogs_url,
	.type      = GLYR_GET_ARTIST_PHOTOS,
	.quality   = 70,
	.speed     = 65,
	.endmarker = NULL,
	.free_url  = false
};
