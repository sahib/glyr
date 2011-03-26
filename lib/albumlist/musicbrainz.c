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

#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include "musicbrainz.h"
#include "../stringlib.h"
#include "../core.h"

const char * albumlist_musicbrainz_url(GlyQuery * sets)
{
	return "http://musicbrainz.org/ws/1/release/?type=xml&artist=%artist%&releasetypes=\"Official\"";
}

#define ALBUM_BEGIN "<release type=\"Album Official\""
#define ALBUM_ENDIN "\" id=\""

#define TITLE_BEGIN "<title>"
#define TITLE_ENDIN "</title>"

GlyCacheList * albumlist_musicbrainz_parse(cb_object * capo)
{
	GlyCacheList * collection = NULL;
	
	char * node = capo->cache->data;
	while( (node = strstr(node+1,ALBUM_BEGIN)) != NULL)
	{
		char * name = copy_value(strstr(node,TITLE_BEGIN) + strlen(TITLE_BEGIN),strstr(node,TITLE_ENDIN) + strlen(TITLE_ENDIN));
		if(name != NULL)
		{
			if(!collection)
			{
				collection = DL_new_lst();
			}
	
			GlyMemCache * c = DL_init();
			c->data = beautify_lyrics(name);

			if(c->data != NULL)
			{
				c->size = strlen(c->data);
			}

			DL_add_to_list(collection,c);
			free(name);
		}
	}
	return collection;
}
