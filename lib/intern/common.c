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

#include "../apikeys.h"
#include "common.h"

/**
* @brief Check if the size of a cover fits the specs
*
* @param sZ
* @param min
* @param max
*
* @return true/false
*/
gboolean size_is_okay(int sZ, int min, int max)
{
    if((min == -1 && max == -1) ||
       (min == -1 && max >= sZ) ||
       (min <= sZ && max == -1) ||
       (min <= sZ && max >= sZ)  )
        return TRUE;

    return FALSE;
}

/*--------------------------------------------------------*/

/* Methods used by more than one provider go here */
const gchar * generic_google_url(GlyrQuery * sets, const gchar * searchterm)
{
    const gchar * lang = NULL;
    if(!strcasecmp(sets->lang,"us"))
        lang = "com"  ;
    else if(!strcasecmp(sets->lang,"ca"))
        lang = "ca"   ;
    else if(!strcasecmp(sets->lang,"uk"))
        lang = "co.uk";
    else if(!strcasecmp(sets->lang,"fr"))
        lang = "fr"   ;
    else if(!strcasecmp(sets->lang,"de"))
        lang = "de"   ;
    else if(!strcasecmp(sets->lang,"jp"))
        lang = "co.jp";
    else
        lang = "com";

    const gchar * back = NULL;
    if(sets->img_min_size == -1)
    {
        back = "";
    }
    else if(sets->img_min_size < 75)
    {
        back = "&tbs=isz:i";
    }
    else if(sets->img_min_size < 300)
    {
        back = "&tbs=isz:m";
    }
    else if(sets->img_min_size < 450)
    {
        back = "&tbs=isz:lt,islt:qsvga";
    }
    else if(sets->img_min_size < 550)
    {
        back = "&tbs=isz:lt,islt:vg/a";
    }
    else if(sets->img_min_size < 700)
    {
        back = "&tbs=isz:lt,islt:svga";
    }
    else /* High enough. */
    {
        back = "&tbs=isz:lt,islt:xga";
    }

    return g_strdup_printf("http://www.google.%s/images?q=%s&safe=off%s%s",lang,searchterm,(back!=NULL) ? "" : "&", back);
}

/*--------------------------------------------------------*/

#define IMG_SRC_START "&amp;usg="
#define WIDTH_START   "&amp;w="
#define HEIGHT_START  "&amp;h="
#define MAX_NUM_BUF   16

static gint google_get_size_value(gchar * ref, gchar * name)
{
    gint number = 0;
    gchar * start = g_strstr_len(ref,256,name);
    if(start != NULL)
    {
        start += strlen(name);
        gchar * end = strchr(start,' ');
        if(end != NULL)
        {
            gchar numbuf[MAX_NUM_BUF] = {};
            gsize span = MIN(end - start,MAX_NUM_BUF-1);
            strncpy(numbuf,start,span);
            number = strtol(numbuf,NULL,10);
        }
    }
    return number;
}

/*--------------------------------------------------------*/

static gboolean google_check_image_size(GlyrQuery * s, gchar * ref)
{
    gboolean result = FALSE;
    gchar * img_src_after = strstr(ref,IMG_SRC_START);
    if(img_src_after != NULL)
    {
        gint width  = google_get_size_value(img_src_after,WIDTH_START);
        gint height = google_get_size_value(img_src_after,HEIGHT_START);
        gint ratio  = (width+height)/2;

        result = size_is_okay(ratio,s->img_min_size,s->img_max_size);
    }
    return result;
}

/*--------------------------------------------------------*/

#define FIRST_RESULT "<a href=\"/imgres?imgurl="
#define END_OF_URL "&amp;imgrefurl="
GList * generic_google_parse(cb_object * capo)
{
    GList * result_list = NULL;
    gchar * find = capo->cache->data;

    while(continue_search(g_list_length(result_list),capo->s) && (find =  strstr(find+1,FIRST_RESULT)) != NULL)
    {
        gchar * end_of_url = NULL;
        find += strlen(FIRST_RESULT);
        if((end_of_url = strstr(find, END_OF_URL)) != NULL)
        {
            if(google_check_image_size(capo->s,find) == TRUE)
            {
                gchar * url = copy_value(find,end_of_url);
                if(url != NULL)
                {
                    GlyrMemCache * result = DL_init();
                    result->data = url;
                    result->size = end_of_url - find;
                    result_list = g_list_prepend(result_list,result);
                }
            }
        }
    }
    return result_list;
}

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
    const gchar * wrap_a = sets->artist ? "%artist%" : "";
    const gchar * wrap_b = sets->album  ? "%album%"  : "";
    const gchar * wrap_t = sets->title  ? "%title%"  : "";

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

/*--------------------------------------------------------*/

#define ACCESS_KEY API_KEY_AMAZON
#define rg_markup "__RESPONSE_GROUP__"
const gchar * generic_amazon_url(GlyrQuery * sets, const gchar * response_group)
{
	const char * lang_link = NULL;
	if(sets->img_min_size <= 500 || sets->img_min_size)
	{
		if(!strcmp(sets->lang,"us"))
			lang_link = "http://free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=%artist%+%album%\0";
		else if(!strcmp(sets->lang,"ca"))
			lang_link = "http://ca.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=%artist%+%album%\0";
		else if(!strcmp(sets->lang,"uk"))
			lang_link = "http://co.uk.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=%artist%+%album%\0";
		else if(!strcmp(sets->lang,"fr"))
			lang_link = "http://fr.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=%artist%+%album%\0";
		else if(!strcmp(sets->lang,"de"))
			lang_link = "http://de.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=%artist%+%album%\0";
		else if(!strcmp(sets->lang,"jp"))
			lang_link = "http://co.jp.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=%artist%+%album%\0";
		else
			lang_link = "http://free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=%artist%+%album%\0";
	}
	if(lang_link != NULL)
	{
		return strreplace(lang_link,rg_markup,response_group);
	}
	return NULL;
}

/* ------------------------------------------------------------- */
