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
#include "picsearch.h"

const gchar * generic_picsearch_url(GlyrQuery * s, const char * fmt)
{
	/* picsearch offers a nice way to set img_min / img_max */
	gchar * base_url = "http://www.picsearch.com/index.cgi?q=%s&face=both&orientation=square&size=%dt%d";
	
	gint img_min_size = s->img_min_size;
	if(img_min_size == -1)
	{
		img_min_size = 0;
	}

 	gint img_max_size = s->img_max_size;
	if(img_max_size == -1)
	{
		img_max_size = INT_MAX;
	}

	return g_strdup_printf(base_url, fmt, img_min_size, img_max_size);
}

/* ------------------------- */

#define IMG_HOOK "<span class=\"key\">Image URL:</span>"

static GlyrMemCache * parse_details_page(GlyrMemCache * to_parse)
{
	GlyrMemCache * result = NULL;
	if(to_parse != NULL)
	{
		gchar * hook = strstr(to_parse->data,IMG_HOOK);
		if(hook != NULL)
		{
			gchar * img_url = get_search_value(hook,"href=\"","\">");
			if(img_url != NULL)
			{
				result = DL_init();
				result->data = img_url;
				result->size = strlen(img_url);
				result->dsrc = g_strdup(to_parse->dsrc);
			}
		}
	}
	return result;
}

/* ------------------------- */

#define NODE "<div class=\"imgContainer\">"

GList * generic_picsearch_parse(cb_object * capo)
{
	GList * result_list = NULL;

	gchar * node = capo->cache->data;
	gint nodelen = (sizeof NODE) - 1;
	while(continue_search(g_list_length(result_list),capo->s) && (node = strstr(node, NODE)))
	{
		node += nodelen;
		gchar * details_url = get_search_value(node,"<a href=\"","\" ");
		if(details_url != NULL)
		{
			GlyrMemCache * to_parse = download_single(details_url,capo->s,NULL);
			if(to_parse != NULL)
			{
				GlyrMemCache * result = parse_details_page(to_parse);
				if(result != NULL)
				{
					result_list = g_list_prepend(result_list,result);
				}
				DL_free(to_parse);
			}
			g_free(details_url);
		}
	}
	return result_list;
}
