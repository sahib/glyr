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
#include "google.h"
#include "../../core.h"
#include "../../stringlib.h"
#include "../../utils.h"

/* ------------------------- */

#define FIRST_RESULT "<a href=\"/imgres?imgurl="
#define END_OF_URL "&amp;imgrefurl="

const char * generic_google_url(GlyrQuery * sets, const char * searchterm)
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

/* ------------------------- */

#define IMG_SRC_START "&amp;usg="
#define WIDTH_START   "&amp;w="
#define HEIGHT_START  "&amp;h="
#define MAX_NUM_BUF   16

static gint get_value(gchar * ref, gchar * name)
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

/* ------------------------- */

static gboolean check_image_size(GlyrQuery * s, gchar * ref)
{
	gboolean result = FALSE;
	gchar * img_src_after = strstr(ref,IMG_SRC_START);
	if(img_src_after != NULL)
	{
		gint width  = get_value(img_src_after,WIDTH_START);
		gint height = get_value(img_src_after,HEIGHT_START);
		gint ratio  = (width+height)/2;

		result = size_is_okay(ratio,s->img_min_size,s->img_max_size);
	}
	return result;
}

/* ------------------------- */

const char * cover_google_url(GlyrQuery * s)
{
    const gchar * result = NULL;
    gchar * searchterm = g_strdup("%artist%+%album%+album");
    if(searchterm != NULL)
    {
        result = generic_google_url(s,searchterm);
        g_free(searchterm);
    }
    return result;
}

/* ------------------------- */

GList * generic_google_parse(cb_object * capo)
{
	GList * r_list = NULL;
	gchar * find = capo->cache->data;
	gsize urlc = 0;

	while( (find =  strstr(find+1,FIRST_RESULT)) != NULL && continue_search(urlc,capo->s))
	{
		gchar * end_of_url = NULL;
		find += strlen(FIRST_RESULT);
		if( (end_of_url = strstr(find, END_OF_URL)) != NULL)
		{
			if(check_image_size(capo->s,find) == TRUE)
			{
				gchar * url = copy_value(find,end_of_url);
				if(url != NULL)
				{
					GlyrMemCache * result = DL_init();
					result->data = url;
					result->size = end_of_url - find;

					r_list = g_list_prepend(r_list,result);
					urlc++;
				}
			}
		}
	}
	return r_list;
}

/* ------------------------- */

GList * cover_google_parse(cb_object * capo)
{
	return generic_google_parse(capo);
}

/* ------------------------- */

MetaDataSource cover_google_src =
{
	.name      = "google",
	.key       = 'g',
	.parser    = cover_google_parse,
	.get_url   = cover_google_url,
	.type      = GET_COVERART,
	.quality   = 35,
	.speed     = 99,
	.endmarker = NULL,
	.free_url  = true
};
