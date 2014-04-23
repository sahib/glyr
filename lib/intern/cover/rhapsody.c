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

#include "../../core.h"
#include "../../stringlib.h"

/////////////////////////////////

static gchar * translate_string (gchar * string)
{
    gchar * result = NULL;
    if (string != NULL)
    {
        gchar * beautiful = beautify_string (string);
        if (beautiful != NULL)
        {
            gchar * downcase = g_utf8_strdown (beautiful,-1);
            if (downcase != NULL)
            {
                result = translate_umlauts (downcase);
                g_free (downcase);
            }
            g_free (beautiful);
        }
    }
    return result;
}

/////////////////////////////////

static const gchar * cover_rhapsody_url (GlyrQuery * query)
{
    gchar * result = NULL;
    gchar * corrected_artist = translate_string (query->artist);
    gchar * corrected_album  = translate_string (query->album);
    if (corrected_artist && corrected_album)
    {
        result = g_strdup_printf ("http://feeds.rhapsody.com/%s/%s/data.xml",corrected_artist,corrected_album);
    }
    g_free (corrected_artist);
    g_free (corrected_album);
    return result;
}

/////////////////////////////////

static gboolean check_size (GlyrQuery * s, gchar * ref)
{
    gboolean result = FALSE;
    if (ref != NULL)
    {
        gchar * width_str  = get_search_value (ref,"width=\"","\"");
        gchar * height_str = get_search_value (ref,"height=\"","\"");
        if (width_str && height_str)
        {
            gint width  = strtol (width_str, NULL,10);
            gint height = strtol (height_str,NULL,10);
            result = size_is_okay ( (width+height) /2,s->img_min_size,s->img_max_size);
        }
        g_free (width_str);
        g_free (height_str);
    }
    return result;
}

/////////////////////////////////

#define DELIM_BEG "<art>"
#define DELIM_END "</art>"
#define NODE "<img "
#define HACK_SIZE "170x170"
#define HIGH_SIZE "500x500"

static GList * cover_rhapsody_parse (cb_object * capo)
{
    GList * result_list = NULL;
    gchar * delim_beg = strstr (capo->cache->data,DELIM_BEG);
    gchar * delim_end = strstr (capo->cache->data,DELIM_END);
    GlyrMemCache * special_size = NULL;

    if (delim_beg && delim_end)
    {
        gchar * node = delim_beg;
        gsize nd_len = (sizeof NODE) - 1;
        while (continue_search (g_list_length (result_list),capo->s) && (node = strstr (node + nd_len, NODE) ) && node < delim_end)
        {
            node += nd_len;
            if (check_size (capo->s,node) == TRUE)
            {
                gchar * url = get_search_value (node,"src=\"","\"");
                if (url != NULL)
                {
                    GlyrMemCache * result = DL_init();
                    result->data = url;
                    result->size = strlen (url);
                    result_list = g_list_prepend (result_list,result);

                    /* A very cool hack. Thanks Bansheeproject! */
                    if (strstr (result->data,HACK_SIZE) != NULL)
                    {
                        special_size = result;
                    }
                }
            }
        }

        /* Awesome hack continues.. */
        if (special_size != NULL)
        {
            /* Note: Prepend the large size at begin:
             * If only one item requested it will just
             * be thrown away (the small size)
             */
            GlyrMemCache * result = DL_init();
            result->data = strreplace (special_size->data,HACK_SIZE,HIGH_SIZE);
            result->size = strlen (result->data);
            result_list = g_list_prepend (result_list,result);
        }
    }
    return result_list;
}

/////////////////////////////////

MetaDataSource cover_rhapsody_src =
{
    .name      = "rhapsody",
    .key       = 'r',
    .parser    = cover_rhapsody_parse,
    .get_url   = cover_rhapsody_url,
    .type      = GLYR_GET_COVERART,
    .quality   = 50,
    .speed     = 80,
    .endmarker = NULL,
    .free_url  = true
};
