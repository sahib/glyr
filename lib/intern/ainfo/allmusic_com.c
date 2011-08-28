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
#include "../../glyr.h"

const char * ainfo_allmusic_url(GlyrQuery * s)
{
    return "http://www.allmusic.com/search/artist/%artist%";
}

/*-------------------------------------*/

#define IMG_BEGIN "<p class=\"text\">"
#define IMG_ENDIN "</p>"

GlyrMemCache * parse_bio_page(GlyrMemCache * to_parse)
{
    GlyrMemCache * result = NULL;
    gchar * text_begin = strstr(to_parse->data,IMG_BEGIN);
    if(text_begin != NULL)
    {
        gchar * text_endin = strstr(text_begin,IMG_ENDIN);
        if(text_endin != NULL)
        {
            gchar * text = copy_value(text_begin + (sizeof IMG_BEGIN) - 1,text_endin);
            if(text != NULL)
            {
                result = DL_init();
                result->data = text;
                result->size = strlen(result->data);
            }
        }
    }
    return result;
}

/*-------------------------------------*/

#define ROOT "<div id=\"tabs\">"
#define ROOT_URL "http://www.allmusic.com/artist/"
#define END_OF_ROOT "\">"

GlyrMemCache * find_long_version(GlyrQuery * s, GlyrMemCache * to_parse)
{
    GlyrMemCache * rche = NULL;
    char * root = strstr(to_parse->data,ROOT);
    if(root)
    {
        char * url_root = strstr(root,ROOT_URL);
        if(url_root != NULL)
        {
            char * id = copy_value(url_root + strlen(ROOT_URL),strstr(url_root,END_OF_ROOT));
            if(id != NULL)
            {
                char * url = g_strdup_printf(ROOT_URL"%s",id);
                if(url != NULL)
                {
                    GlyrMemCache * dl = download_single(url,s,NULL);
                    if(dl != NULL)
                    {
                        rche = parse_bio_page(dl);
                        DL_free(dl);
                    }
                    g_free(url);
                }
                g_free(id);
            }
        }
    }
    return rche;
}

/*-------------------------------------*/
#define SEARCH_TREE_BEGIN "<table class=\"search-results\""
#define SEARCH_NODE "<td><a href=\""
#define SEARCH_DELIM "\">"
#define END_OF_ARTIST "</a>"

static gboolean approve_content(GlyrQuery * query, gchar * ref)
{
	gboolean result = FALSE;
	if(ref != NULL)
	{
			ref += (sizeof SEARCH_DELIM) - 1;
			gchar * end_of_artist = strstr(ref,END_OF_ARTIST);
			if(end_of_artist != NULL)
			{
					gchar * artist_html = copy_value(ref, end_of_artist);
					if(artist_html != NULL)
					{
						if(levenshtein_strnormcmp(artist_html,query->artist) <= query->fuzzyness)
						{
								result = TRUE;
						}
						g_free(artist_html);
					}
			}
	}
	return result;
}

/*-------------------------------------*/

GList * ainfo_allmusic_parse(cb_object * capo)
{
    GList * result_list = NULL;

	/* Are we already on the biopage? */
    if(strstr(capo->cache->data, "<!--Begin Biography -->"))
    {
        GlyrMemCache * info_long = find_long_version(capo->s,capo->cache);
        result_list = g_list_prepend(result_list,info_long);
        return result_list;
  	}

    gchar * search_begin = NULL;

	/* Hello, anybody there? */
    if((search_begin = strstr(capo->cache->data, SEARCH_TREE_BEGIN)) == NULL)
    {
        return NULL;
    }

    gchar * node = search_begin;
    while(continue_search(g_list_length(result_list),capo->s) && (node = strstr(node+1,SEARCH_NODE)))
    {
		gchar * end_of_url = strstr(node,SEARCH_DELIM);
		if(approve_content(capo->s,end_of_url) == TRUE)
		{
				gchar * url = copy_value(node+(sizeof SEARCH_NODE) - 1, end_of_url);
				if(url != NULL)
				{
					gchar * biography_url = g_strdup_printf("%s/biography",url);
					GlyrMemCache * dl_cache = download_single(biography_url,capo->s,NULL);
					if(dl_cache != NULL)
					{
						GlyrMemCache * content = parse_bio_page(dl_cache);
						if(content != NULL)
						{
							result_list = g_list_prepend(result_list,content);
						}
						DL_free(dl_cache);
					}
					g_free(biography_url);
					g_free(url);
				}
		}
    }
    return result_list;
}

/*-------------------------------------*/

MetaDataSource ainfo_allmusic_src =
{
    .name      = "allmusic",
    .key       = 'm',
    .free_url  = false,
    .type      = GLYR_GET_ARTISTBIO,
    .parser    = ainfo_allmusic_parse,
    .get_url   = ainfo_allmusic_url,
    .quality   = 70,
    .speed     = 40,
    .endmarker = NULL
};
