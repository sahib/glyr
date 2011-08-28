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

const gchar * albumlist_musicbrainz_url(GlyrQuery * sets)
{
    return "http://musicbrainz.org/ws/1/release/?type=xml&artist=%artist%&releasetypes=\"Official\"";
}

#define ALBUM_BEGIN "<release type=\"Album Official\""
#define ALBUM_ENDIN "\" id=\""

#define TITLE_BEGIN "<title>"
#define TITLE_ENDIN "</title>"

GList * albumlist_musicbrainz_parse(cb_object * capo)
{
    GList * collection = NULL;
    gchar * node = capo->cache->data;

    while(continue_search(g_list_length(collection),capo->s) && (node = strstr(node+1,ALBUM_BEGIN)) != NULL)
    {
		gchar * title_begin = strstr(node,TITLE_BEGIN);
		gchar * title_endin = strstr(node,TITLE_ENDIN);
		if(title_begin != NULL && title_endin != NULL)
		{
			gsize title_beg_len = (sizeof TITLE_BEGIN) - 1;
			gsize title_end_len = (sizeof TITLE_ENDIN) - 1;
        	gchar * name = copy_value(title_begin + title_beg_len, title_endin + title_end_len);
        	if(name != NULL)
        	{
            	GlyrMemCache * c = DL_init();
            	c->data = beautify_string(name);
            	c->size = (c->data) ? strlen(c->data) : 0;
            	collection = g_list_prepend(collection,c);
            	g_free(name);
			}
        }
    }
    return collection;
}

/*-----------------------------*/

MetaDataSource albumlist_musicbrainz_src =
{
    .name = "musicbrainz",
    .key = 'm',
    .free_url = false,
    .parser  = albumlist_musicbrainz_parse,
    .get_url = albumlist_musicbrainz_url,
    .type    = GLYR_GET_ALBUMLIST,
    .quality = 95,
    .speed   = 95,
    .endmarker = NULL
};
