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
#include "../../apikeys.h"
#include "../../stringlib.h"

#define HTBACKDROPS_URL "http://htbackdrops.org/api/%s"    \
    "/searchXML?keywords=${artist}&default_operator=and&"  \
    "fields=title&inc=mb_name&limit=%d&dmax_w=%d&dmin_h=%d"

static const gchar * backdrops_htbackdrops_url (GlyrQuery * q)
{
    gchar * result = g_strdup_printf (HTBACKDROPS_URL, API_KEY_HTBACK,q->number * 20,
                                      (q->img_max_size < 0) ? (gint) 1e10 : q->img_max_size,
                                      (q->img_min_size < 0) ? 0          : q->img_min_size);

    return result;
}

/////////////////////////////////

static gboolean check_size (GlyrQuery * query, const gchar * size_string)
{
    gboolean result = FALSE;
    if (size_string != NULL && query != NULL)
    {
        /* "1024x1024" -> ["1024","1024"] */
        gchar ** strv = g_strsplit (size_string,"x",0);
        if (strv && strv[0] && strv[1])
        {
            gint width  = strtol (strv[0],NULL,10);
            gint height = strtol (strv[1],NULL,10);
            gint img_size = (width + height) / 2;

            result = size_is_okay (img_size, query->img_min_size, query->img_max_size);
        }
        g_strfreev (strv);
    }
    return result;
}

/////////////////////////////////

#define NODE "<image>"

static GList * backdrops_htbackdrops_parse (cb_object * capo)
{
    GList * result_list = NULL;
    gchar * img_list_start = strstr (capo->cache->data,"<images>");
    if (img_list_start != NULL)
    {
        gchar * node = img_list_start;
        while (continue_search (g_list_length (result_list),capo->s) && (node = strstr (node,NODE) ) )
        {
            node += sizeof NODE;

            gchar * dimensions = get_search_value (node,"<dimensions>","</dimensions>");
            if (check_size (capo->s,dimensions) == TRUE)
            {
                gchar * validate_artist = get_search_value (node,"<mb_name>","</mb_name>");
                if (levenshtein_strnormcmp (capo->s,validate_artist,capo->s->artist) <= capo->s->fuzzyness)
                {
                    gchar * id = get_search_value (node,"<id>","</id>");
                    if (id != NULL)
                    {
                        GlyrMemCache * result = DL_init();
                        result->data = g_strdup_printf ("http://htbackdrops.org/api/"API_KEY_HTBACK"/download/%s/fullsize",id);
                        result->size = strlen (result->data);
                        result_list  = g_list_prepend (result_list,result);
                        g_free (id);
                    }
                }
                g_free (validate_artist);
            }
            g_free (dimensions);
        }
    }
    return result_list;
}

/////////////////////////////////

MetaDataSource backdrops_htbackdrops_src =
{
    .name = "htbackdrops",
    .key  = 'h',
    .parser    = backdrops_htbackdrops_parse,
    .get_url   = backdrops_htbackdrops_url,
    .type      = GLYR_GET_BACKDROPS,
    .quality   = 80,
    .speed     = 80,
    .endmarker = NULL,
    .free_url  = true
};
