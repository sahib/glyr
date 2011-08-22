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
static int please_what_type(GlyrQuery * s)
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
const char * generic_musicbrainz_url(GlyrQuery * sets)
{
    const gchar * wrap_a = sets->artist ? "%artist%" : "";
    const gchar * wrap_b = sets->album  ? "%album%"  : "";
    const gchar * wrap_t = sets->title  ? "%title%"  : "";

    switch(please_what_type(sets))
    {
    case TYPE_TAG_TITLE :
        return g_strdup_printf("http://musicbrainz.org/ws/1/track/?type=xml&title=%s&artist=%s&release=%s",wrap_t,wrap_a,wrap_b);
    case TYPE_TAG_ALBUM :
        return g_strdup_printf("http://musicbrainz.org/ws/1/release/?type=xml&title=%s&artist=%s",wrap_b,wrap_a);
    case TYPE_TAG_ARTIST:
        return g_strdup_printf("http://musicbrainz.org/ws/1/artist/?type=xml&name=%s",wrap_a);
    default:
        return NULL;
    }
}

/*--------------------------------------------------------*/

#define ID_BEGIN "id=\""

const gchar * get_mbid_from_xml(GlyrQuery * s, GlyrMemCache * c, gint * offset)
{
    if(c==NULL || s==NULL || offset==NULL)
        return NULL;

    const gchar * searchterm  = NULL;
    const gchar * checkstring = NULL;
    const gchar * comparestr  = NULL;

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
        glyr_message(1,s,"Warning: (tags/musicbrainz.c) Unable to determine type.\n");
    }

    const gchar * mbid = NULL;
    if(searchterm != NULL)
    {
        gchar * node = c->data + *offset;
        gchar * search_check = NULL;
        gsize nlen = (sizeof ID_BEGIN) - 1;
        gsize clen = strlen(checkstring);

        while( (node = strstr(node,searchterm)) && mbid == NULL)
        {
            if(!(node = strstr(node,ID_BEGIN)))
                continue;

            if(!(search_check = strstr(node,checkstring)))
                continue;

			search_check += clen;
            gchar * to_compare = copy_value(search_check,strstr(search_check,"</"));
            if(to_compare != NULL)
            {
				gchar * escaped = beautify_lyrics(to_compare);
				if(escaped != NULL)
				{
						if(levenshtein_strcmp(escaped,comparestr) < s->fuzzyness)
						{
								mbid = (gchar*)copy_value(node+nlen,strchr(node+nlen,'"'));
						}
						g_free(escaped);
				}
				g_free(to_compare);
			}
			node += (sizeof ID_BEGIN) - 1;
		}
		*offset = node - c->data;
	}
	return mbid;
}

/*--------------------------------------------------------*/

// Returns only a parseable memcache
GlyrMemCache * generic_musicbrainz_parse(cb_object * capo, gint * last_mbid, const gchar * include)
{
		gsize offset = 0;
		const gchar * mbid = NULL;
		GlyrMemCache * info = NULL;

		while(offset < capo->cache->size && info==NULL && (mbid = get_mbid_from_xml(capo->s,capo->cache,last_mbid)))
		{
				if(mbid != NULL)
				{
						const gchar * type = NULL;
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

						gchar * info_page_url = g_strdup_printf("http://musicbrainz.org/ws/1/%s/%s?type=xml&inc=%s",type,mbid,include);
						if(info_page_url)
						{
								info = download_single(info_page_url,capo->s,NULL);
								g_free(info_page_url);
						}
						g_free((gchar*)mbid);
				}
		}
		return info;
}

/*--------------------------------------------------------*/

/* Wrap around the (a bit more) generic versions */
GList * tags_musicbrainz_parse(cb_object * capo)
{
		GList * results = NULL;
		gint mbid_marker = 0;
		while(continue_search(g_list_length(results), capo->s))
		{
				GlyrMemCache  * info = generic_musicbrainz_parse(capo,&mbid_marker,"tags");
				if(info == NULL)
				{
						break;
				}
				gint type_num = please_what_type(capo->s);
				gchar * tag_node = info->data;
				while( (tag_node = strstr(tag_node + 1,"<tag")) )
				{
						gchar * tag_begin = strchr(tag_node+1,'>');
						if(!tag_begin)
								continue;

						tag_begin++;
						gchar * tag_endin = strchr(tag_begin,'<');
						if(!tag_endin)
								continue;

						gchar * value = copy_value(tag_begin,tag_endin);
						if(value != NULL)
						{
								if(strlen(value) > 0)
								{
										GlyrMemCache * tmp = DL_init();
										tmp->data = value;
										tmp->size = tag_endin - tag_begin;
										tmp->type = type_num;

										results = g_list_prepend(results,tmp);
								}
						}
				}
				DL_free(info);
		}
		return results;
}

/*--------------------------------------------------------*/

const char * tags_musicbrainz_url(GlyrQuery * sets)
{
		return generic_musicbrainz_url(sets);
}

/*--------------------------------------------------------*/

MetaDataSource tags_musicbrainz_src =
{
		.name = "musicbrainz",
		.key  = 'm',
		.parser    = tags_musicbrainz_parse,
		.get_url   = tags_musicbrainz_url,
		.endmarker = NULL,
		.free_url  = true,
		.quality   = 90,
		.speed     = 90,
		.type      = GET_TAGS
};
