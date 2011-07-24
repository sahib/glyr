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


/* Possible cases:
------------------
artist -> artist
album  -> album
title  -> title
---------------
artist && album && title -> title
artist && title          -> title
artist && album          -> album
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

/*--------------------------------------------------------*/

/*  */
const char * generic_musicbrainz_url(GlyQuery * sets)
{
    const char * wrap_a = sets->artist ? "%artist%" : "";
    const char * wrap_b = sets->album  ? "%album%"  : "";
    const char * wrap_t = sets->title  ? "%title%"  : "";

    switch(please_what_type(sets))
    {
    case TYPE_TAG_TITLE :
        return strdup_printf("http://musicbrainz.org/ws/1/track/?type=xml&title=%s&artist=%s&release=%s",wrap_t,wrap_a,wrap_b);
    case TYPE_TAG_ALBUM :
        return strdup_printf("http://musicbrainz.org/ws/1/release/?type=xml&title=%s&artist=%s",wrap_b,wrap_a);
    case TYPE_TAG_ARTIST:
        return strdup_printf("http://musicbrainz.org/ws/1/artist/?type=xml&name=%s",wrap_a);
    default:
        return NULL;
    }
}

/*--------------------------------------------------------*/

#define ID_BEGIN "id=\""

const char * get_mbid_from_xml(GlyQuery * s, GlyMemCache * c, size_t * offset)
{
    if(!c || !s || !offset)
        return NULL;

    const char * searchterm  = NULL;
    const char * checkstring = NULL;
    const char * comparestr  = NULL;

    switch(please_what_type(s))
    {
    case TYPE_TAG_TITLE:
        checkstring = "<title>";
        searchterm  = "<track ";
        comparestr  = s->title;
        break;
    case TYPE_TAG_ALBUM:
        checkstring = "<title>";
        searchterm  = "<release ";
        comparestr  = s->album;
        break;
    case TYPE_TAG_ARTIST:
        checkstring = "<name>";
        searchterm  = "<artist ";
        comparestr  = s->artist;
        break;
    default:
        glyr_message(0,s,stderr,"Warning: (tags/musicbrainz.c) Unable to determine type.\n");
    }

    const char * mbid = NULL;
    if(searchterm != NULL)
    {
        char * node = c->data + *(offset);
        char * search_check = NULL;
        size_t nlen = strlen(ID_BEGIN);
        size_t clen = strlen(checkstring);

        while( node && (node = strstr(node,searchterm)) && !mbid)
        {
            if( !(node = strstr(node,ID_BEGIN)) )
                continue;

            if(! (search_check = strstr(node,checkstring)) )
                continue;

            char * to_compare = copy_value(search_check+clen,strstr(search_check+clen,"</"));
            if(to_compare != NULL)
            {
                if(levenshtein_strcmp(to_compare,comparestr) < s->fuzzyness)
                {
                    mbid = (char*)copy_value(node+nlen,strchr(node+nlen,'"'));
                    if(mbid && !(*mbid))
                    {
                        free((char*)mbid);
                        mbid = NULL;
                    }
                }
                free(to_compare);
            }
            node++;
        }
        *offset = node - c->data;
    }
    return mbid;
}

/*--------------------------------------------------------*/

// Returns only a parseable memcache
GlyMemCache * generic_musicbrainz_parse(cb_object * capo, const char * include)
{
    size_t offset = 0;
    const char * mbid = NULL;
    GlyMemCache * info = NULL;

    int ctr = 0;

    while(continue_search(ctr,capo->s) && !info && (mbid = get_mbid_from_xml(capo->s,capo->cache,&offset)) )
    {
        if(mbid != NULL)
        {
            const char * type = NULL;
            switch(please_what_type(capo->s))
            {
            case TYPE_TAG_TITLE:
                type = "track";
                break;
            case TYPE_TAG_ALBUM:
                type = "release";
                break;
            case TYPE_TAG_ARTIST:
                type = "artist";
                break;
            }

            char * info_page_url = strdup_printf("http://musicbrainz.org/ws/1/%s/%s?type=xml&inc=%s",type,mbid,include);
            if(info_page_url)
            {
                info = download_single(info_page_url,capo->s,NULL);
                free(info_page_url);
            }
            free((char*)mbid);
        }
    }
    return info;
}

/*--------------------------------------------------------*/

/* Wrap around the (a bit more) generic versions */
GlyCacheList * tags_musicbrainz_parse(cb_object * capo)
{
    GlyCacheList * r_list = NULL;
    GlyMemCache  * info = generic_musicbrainz_parse(capo,"tags");
    if(info)
    {
        int type_num = please_what_type(capo->s);
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
                    tmp->dsrc = info->dsrc ? strdup(info->dsrc) : NULL;

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
    return r_list;
}

/*--------------------------------------------------------*/

const char * tags_musicbrainz_url(GlyQuery * sets)
{
    return generic_musicbrainz_url(sets);
}

/*--------------------------------------------------------*/

MetaDataSource tags_musicbrainz_src = {
	.name = "musicbrainz",
	.key  = 'm',
	.parser    = tags_musicbrainz_parse,
	.get_url   = tags_musicbrainz_url,
	.endmarker = NULL, 
	.free_url  = true,
	.type      = GET_TAGS
};
