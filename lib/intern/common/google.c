/***********************************************************
* This file is part of glyr
* + a commnadline tool and library to download various sort of music related metadata.
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
#include "../../stringlib.h"
#include "../../core.h"

/////////////////////////////////

/* Methods used by more than one provider go here */
const gchar * generic_google_url (GlyrQuery * sets, const gchar * searchterm)
{
    const gchar * lang = NULL;
    if (!g_ascii_strncasecmp (sets->lang,"us",2) )
        lang = "com"  ;
    else if (!g_ascii_strncasecmp (sets->lang,"ca",2) )
        lang = "ca"   ;
    else if (!g_ascii_strncasecmp (sets->lang,"uk",2) )
        lang = "co.uk";
    else if (!g_ascii_strncasecmp (sets->lang,"fr",2) )
        lang = "fr"   ;
    else if (!g_ascii_strncasecmp (sets->lang,"de",2) )
        lang = "de"   ;
    else if (!g_ascii_strncasecmp (sets->lang,"jp",2) )
        lang = "co.jp";
    else
        lang = "com";

    const gchar * back = NULL;
    if (sets->img_min_size == -1)
    {
        back = "";
    }
    else if (sets->img_min_size < 75)
    {
        back = "&tbs=isz:i";
    }
    else if (sets->img_min_size < 300)
    {
        back = "&tbs=isz:m";
    }
    else if (sets->img_min_size < 450)
    {
        back = "&tbs=isz:lt,islt:qsvga";
    }
    else if (sets->img_min_size < 550)
    {
        back = "&tbs=isz:lt,islt:vg/a";
    }
    else if (sets->img_min_size < 700)
    {
        back = "&tbs=isz:lt,islt:svga";
    }
    else /* High enough. */
    {
        back = "&tbs=isz:lt,islt:xga";
    }

    return g_strdup_printf ("http://www.google.%s/images?q=%s&safe=off%s%s",lang,searchterm, (back!=NULL) ? "" : "&", back);
}

/////////////////////////////////

#define IMG_SRC_START "&amp;usg="
#define WIDTH_START   "&amp;w="
#define HEIGHT_START  "&amp;h="
#define MAX_NUM_BUF   16

static gint google_get_size_value (gchar * ref, gchar * name)
{
    gint number = 0;
    gchar * start = g_strstr_len (ref,256,name);
    if (start != NULL)
    {
        start += strlen (name);
        gchar * end = strchr (start,' ');
        if (end != NULL)
        {
            gchar numbuf[MAX_NUM_BUF] = {};
            gsize span = MIN (end - start,MAX_NUM_BUF-1);
            strncpy (numbuf,start,span);
            number = strtol (numbuf,NULL,10);
        }
    }
    return number;
}

/////////////////////////////////

static gboolean google_check_image_size (GlyrQuery * s, gchar * ref)
{
    gboolean result = FALSE;
    gchar * img_src_after = strstr (ref,IMG_SRC_START);
    if (img_src_after != NULL)
    {
        gint width  = google_get_size_value (img_src_after,WIDTH_START);
        gint height = google_get_size_value (img_src_after,HEIGHT_START);
        gint ratio  = (width+height) /2;

        result = size_is_okay (ratio,s->img_min_size,s->img_max_size);
    }
    return result;
}

/////////////////////////////////

#define FIRST_RESULT "<a href=\"/imgres?imgurl="
#define END_OF_URL "&amp;imgrefurl="
GList * generic_google_parse (cb_object * capo)
{
    GList * result_list = NULL;
    gchar * find = capo->cache->data;

    while (continue_search (g_list_length (result_list),capo->s) && (find =  strstr (find+1,FIRST_RESULT) ) != NULL)
    {
        gchar * end_of_url = NULL;
        find += strlen (FIRST_RESULT);
        if ( (end_of_url = strstr (find, END_OF_URL) ) != NULL)
        {
            if (google_check_image_size (capo->s,find) == TRUE)
            {
                gchar * url = copy_value (find,end_of_url);
                if (url != NULL)
                {
                    GlyrMemCache * result = DL_init();
                    result->data = url;
                    result->size = end_of_url - find;
                    result_list = g_list_prepend (result_list,result);
                }
            }
        }
    }
    return result_list;
}
