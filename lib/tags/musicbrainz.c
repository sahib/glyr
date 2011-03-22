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
#include "../stringop.h"
#include "../core.h"


/* Possible cases:

artist -> artist
album -> album
title -> title

artist && album && title -> title
artist && title -> title
artist && album -> album
*/

#define _TITLE  0
#define _ALBUM  1
#define _ARTIST 2

// 'please' is important. gcc won't compile without.
static const char * please_what_type(GlyQuery * s)
{
   	if(s->artist && !s->album && !s->title)
	    return _ARTIST;
	else if(!s->artist && !s->album && s->title)
	    return _TITLE;
	else if(!s->artist && s->album && !s->title)
	    return _ALBUM;
	else if(s->artist && s->album && s->title)
	    return _TITLE;
	else if(s->artist && !s->album && s->title)
	    return _TITLE;
	else if(s->artist && s->album && !s->title)
	    return _ALBUM;
	else 
	    return NULL;
}

const char * albumlist_musicbrainz_url(GlyQuery * sets)
{

    char wrap_a = sets->artist ? sets->artist : "";
    char wrap_b = sets->album  ? sets->album  : "";
    char wrap_t = sets->title  ? sets->title  : "";

    switch(please_what_type(sets))
    {
	case _TITLE : return strdup_printf("http://musicbrainz.org/ws/1/artist/?type=xml&name=%artist%",);
	case _ALBUM : return strdup_printf("http://musicbrainz.org/ws/1/review/?type=xml&title=%s&artist=%s",wrap_t,wrap_a);
	case _ARTIST: return strdup_printf("http://musicbrainz.org/ws/1/artist/?type=xml&name=%s",wrap_a);
	default:      return NULL; // No Idea what happened here...
    }
}

// http://musicbrainz.org/ws/1/artist/c0b2500e-0cef-4130-869d-732b23ed9df5?type=xml&inc=url-rels+tags
char * parse_mbid_from_xml(cb_object * capo)
{
	
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
