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

#include "musicbrainz.h"
#include "../../stringlib.h"
#include "../../core.h"

/*--------------------------------------------------------*/

/* 'please' is important. gcc won't compile without. */
gint please_what_type(GlyrQuery * s)
{
    int result = -1;
    if(s->artist && !s->album && !s->title)
        result = GLYR_TYPE_TAG_ARTIST;
    else if(!s->artist && !s->album && s->title)
        result = GLYR_TYPE_TAG_TITLE;
    else if(!s->artist && s->album && !s->title)
        result = GLYR_TYPE_TAG_ALBUM;
    else if(s->artist && s->album && s->title)
        result = GLYR_TYPE_TAG_TITLE;
    else if(s->artist && !s->album && s->title)
        result = GLYR_TYPE_TAG_TITLE;
    else if(s->artist && s->album && !s->title)
        result = GLYR_TYPE_TAG_ALBUM;
    else
        result = -1;

    return result;
}

/*--------------------------------------------------------*/

const gchar * generic_musicbrainz_url(GlyrQuery * sets)
{
    const gchar * wrap_a = sets->artist ? "${artist}" : "";
    const gchar * wrap_b = sets->album  ? "${album}"  : "";
    const gchar * wrap_t = sets->title  ? "${title}"  : "";

    switch(please_what_type(sets))
    {
        case GLYR_TYPE_TAG_TITLE :
            return g_strdup_printf("http://musicbrainz.org/ws/1/track/?type=xml&title=%s&artist=%s&release=%s",wrap_t,wrap_a,wrap_b);
        case GLYR_TYPE_TAG_ALBUM :
            return g_strdup_printf("http://musicbrainz.org/ws/1/release/?type=xml&title=%s&artist=%s",wrap_b,wrap_a);
        case GLYR_TYPE_TAG_ARTIST:
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
        case GLYR_TYPE_TAG_TITLE:
            checkstring = "<title>";
            searchterm  = "<track ";
            comparestr  = s->title;
            break;
        case GLYR_TYPE_TAG_ALBUM:
            checkstring = "<title>";
            searchterm  = "<release ";
            comparestr  = s->album;
            break;
        case GLYR_TYPE_TAG_ARTIST:
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

        while(node && (node = strstr(node,searchterm)) && mbid == NULL)
        {
            if(!(node = strstr(node,ID_BEGIN)))
                break;

            if(!(search_check = strstr(node,checkstring)))
                break;

            search_check += clen;
            gchar * to_compare = copy_value(search_check,strstr(search_check,"</"));
            if(to_compare != NULL)
            {
                if(levenshtein_strnormcmp(s,to_compare,comparestr) <= s->fuzzyness)
                {
                    mbid = (gchar*)copy_value(node+nlen,strchr(node+nlen,'"'));
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

/* Returns only a parseable memcache */
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
                case GLYR_TYPE_TAG_TITLE:
                    type = "track";
                    break;
                case GLYR_TYPE_TAG_ALBUM:
                    type = "release";
                    break;
                case GLYR_TYPE_TAG_ARTIST:
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
