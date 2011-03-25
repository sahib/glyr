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

// 'please' is important. gcc won't compile without.
static int please_what_type(GlyQuery * s)
{
	int result = -1;
   	if(s->artist && !s->album && !s->title)
	    result = TYPE_TAG_ARTIST;
	else if(!s->artist && !s->album && s->title)
	    result = TYPE_TAG_TITLE;
	else if(!s->artist && s->album && !s->title)
	    result = TYPE_TAG_ALBUM;
	else if(s->artist && s->album && s->title)
	    result = TYPE_TAG_TITLE;
	else if(s->artist && !s->album && s->title)
	    result = TYPE_TAG_TITLE;
	else if(s->artist && s->album && !s->title)
	    result = TYPE_TAG_ALBUM;
	else 
	    result = -1;

	return result;
}

const char * tags_musicbrainz_url(GlyQuery * sets)
{

    const char * wrap_a = sets->artist ? "%artist%" : "";
    const char * wrap_b = sets->album  ? "%album%"  : "";
    const char * wrap_t = sets->title  ? "%title%"  : "";

    switch(please_what_type(sets))
    {
	case TYPE_TAG_TITLE : return strdup_printf("http://musicbrainz.org/ws/1/track/?type=xml&title=%s&artist=%s&release=%s",wrap_t,wrap_a,wrap_b);
	case TYPE_TAG_ALBUM : return strdup_printf("http://musicbrainz.org/ws/1/release/?type=xml&title=%s&artist=%s",wrap_b,wrap_a);
	case TYPE_TAG_ARTIST: return strdup_printf("http://musicbrainz.org/ws/1/artist/?type=xml&name=%s",wrap_a);
	default:      return NULL;
    }
}

const char * get_mbid_from_xml(GlyQuery * s, GlyMemCache * c, size_t * offset)
{
    if(!c || !s || !offset)
       return NULL;

    const char * searchterm = NULL;
    switch(please_what_type(s))
    {
	case TYPE_TAG_TITLE:  searchterm = "<track id=\""; break;
	case TYPE_TAG_ALBUM:  searchterm = "<release type=\"Album Official\" id=\""; break;
	case TYPE_TAG_ARTIST: searchterm = "<artist type=\"Group\" id=\""; break;
	default: glyr_message(0,s,stderr,"Warning: (musicbrainz.c) Unable to determine type.\n");
    }
   
    const char * mbid = NULL;
    if(searchterm != NULL)
    {
	char * node = c->data + *(offset);
	size_t nlen = strlen(searchterm);

	while( node && (node = strstr(node,searchterm)) && !mbid)
	{
		mbid = (char*)copy_value(node+nlen,strchr(node+nlen,'"'));
		if(mbid && !(*mbid))
		{
		    free((char*)mbid);
		    mbid = NULL;
		}
		node++;
	}
	
	*offset = node - c->data;
    }
    return mbid;
}

GlyCacheList * tags_musicbrainz_parse(cb_object * capo)
{
	GlyCacheList * r_list = NULL;
	size_t offset = 0; 

	const char * mbid = NULL; 
	while( (mbid = get_mbid_from_xml(capo->s,capo->cache,&offset)) )
	{
		if(mbid != NULL)
		{
			int type_num = please_what_type(capo->s);			

			const char * type = NULL;
			switch(type_num)
			{
			    case TYPE_TAG_TITLE:  type = "track";   break;
			    case TYPE_TAG_ALBUM:  type = "release"; break;
			    case TYPE_TAG_ARTIST: type = "artist";  break;
			}

			char * info_page_url = strdup_printf("http://musicbrainz.org/ws/1/%s/%s?type=xml&inc=tags",type,mbid);
			if(info_page_url)
			{
				GlyMemCache * info = download_single(info_page_url,capo->s,NULL);
				if(info)
				{
					char * tag_node = info->data;
					while( (tag_node = strstr(tag_node + 1,"<tag")) )
					{
						char * tag_begin = strchr(tag_node+1,'>');
						if(!tag_begin)					
						  continue;

						char * tag_endin = strchr(tag_begin,'<');
						if(!tag_endin)
						  continue;

						char * value = copy_value(++tag_begin,tag_endin);
						if(value)
						{
							if(*value)
							{
								GlyMemCache * tmp = DL_init();
								tmp->data = value;
								tmp->size = tag_endin - tag_begin;
								tmp->type = type_num;
	
								if(!r_list) r_list = DL_new_lst();
								DL_add_to_list(r_list,tmp);
							}
							else
							{
								free(value);
							}
						}
					}
					DL_free(info);
				}
				free(info_page_url); 
			}
			free((char*)mbid);
		}
	}
        return r_list;
}
