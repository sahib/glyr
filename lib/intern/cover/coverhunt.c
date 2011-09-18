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

/*
 * Coverhunt seems to have some load problems at time of writing...
 */

static const gchar * cover_coverhunt_url(GlyrQuery * sets)
{
	if(sets->img_min_size <= 500 || sets->img_min_size == -1)
	{
		return "http://www.coverhunt.com/index.php?query=${artist}+${album}&action=Find+my+CD+Covers";
	}
	return NULL;
}

static gboolean check_size(const char * art_root, const char *hw, cb_object * capo)
{
	gchar * begin = strstr(art_root,hw);
	if(begin != NULL)
	{
		gchar * end = strchr(begin,' ');
		gchar * buf = copy_value(begin+strlen(hw),end);
		if(buf != NULL)
		{
			gint atoid = strtol(buf,NULL,10);
			g_free(buf);

			if((atoid >= capo->s->img_min_size || capo->s->img_min_size == -1) &&
		 	   (atoid <= capo->s->img_max_size || capo->s->img_max_size == -1)  )
				return TRUE;
		}
	}
	return FALSE;
}

#define SEARCH_RESULT_BEGIN "<table><tr><td"
#define IMG_START "<img src=\""
#define NODE_BEGIN "<a href=\"/go/"

/* Take the first link we find.
 * coverhunt sadly offers  no way to check if the
 * image is really related to the query we're searching for
 */
static GList * cover_coverhunt_parse(cb_object *capo)
{
	GList * result_list = NULL;

	/* navigate to start of search results */
	gchar * table_start;
	if( (table_start = strstr(capo->cache->data,SEARCH_RESULT_BEGIN)) == NULL)
	{
		/* Whoops, nothing to see here */
		return NULL;
	}

	while(continue_search(g_list_length(result_list),capo->s) && (table_start = strstr(table_start + 1,NODE_BEGIN)))
	{
		gchar * table_end = NULL;
		if( (table_end = strstr(table_start,"\">")) != NULL)
		{
			gchar * go_url = copy_value(table_start + strlen(NODE_BEGIN),table_end);
			if(go_url)
			{
				gchar * real_url = g_strdup_printf("http://www.coverhunt.com/go/%s",go_url);
				if(real_url != NULL)
				{
					GlyrMemCache * search_buf = download_single(real_url,capo->s,"<div id=\"right\">");
					if(search_buf != NULL)
					{
						gchar * artwork = strstr(search_buf->data, "<div class=\"artwork\">");
						if(artwork != NULL)
						{
							if(check_size(artwork,"height=",capo) && check_size(artwork,"width=",capo))
							{
								gchar * img_start = strstr(artwork,IMG_START);
								if(img_start != NULL)
								{
									img_start += (sizeof IMG_START) - 1;
									gchar * img_end = strstr(img_start,"\" ");
									if(img_end != NULL)
									{
										gchar * url = copy_value(img_start,img_end);
										if(url != NULL)
										{
											GlyrMemCache * shell = DL_init();
											shell->data = url;
											shell->size = img_end - img_start;
											shell->dsrc = g_strdup(real_url);
											result_list = g_list_prepend(result_list,shell);
										}
									}
								}
							}
						}
						DL_free(search_buf);
					}
					g_free(real_url);
				}
				g_free(go_url);
			}
		}
	}
	return result_list;
}

/* Queued last, as long coverhunt is down */
MetaDataSource cover_coverhunt_src =
{
	.name      = "coverhunt",
	.key       = 'c',
	.parser    = cover_coverhunt_parse,
	.get_url   = cover_coverhunt_url,
	.type      = GLYR_GET_COVERART,
	.endmarker = "<div id=\"footer\">",
	.quality   = 0, /* ex. 70 */
	.speed     = 0, /* ex. 40 */
	.free_url  = false
};
