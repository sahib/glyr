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
#include "../../core.h"
#include "../../stringlib.h"

#define LINE_BEGIN "<photo id="
#define LINE_ENDIN "/>"

/////////////////////////////////

static const gchar * photos_flickr_url (GlyrQuery * settings)
{
    if (settings->img_max_size <= 175 && settings->img_max_size != -1)
    {
        return NULL;
    }

    gchar * url = g_strdup_printf ("http://api.flickr.com/services/rest/"
                                   "?method=flickr.photos.search&"
                                   "api_key="API_KEY_FLICKR"&"
                                   "tags=%s&"
                                   "media=photos&"
                                   "group_id=29928242@N00&"
                                   "content_type=6&"
                                   "sort=interestingness-asc&"
                                   "per_page=%d",
                                   settings->artist,
                                   settings->number
                                  );
    return url;
}

/////////////////////////////////

static GList * photos_flickr_parse (cb_object * capo)
{
    gchar * ph_begin = capo->cache->data;
    GList * result_list = NULL;

    while (continue_search (g_list_length (result_list),capo->s) && (ph_begin=strstr (ph_begin,LINE_BEGIN) ) != NULL)
    {
        gchar * ph_end = strstr (ph_begin,LINE_ENDIN);
        if (ph_end != NULL)
        {
            gchar * linebf = copy_value (ph_begin,ph_end);
            if (linebf != NULL)
            {
                gchar * ID = get_search_value (linebf, "id=\"","\"");
                gchar * SC = get_search_value (linebf, "secret=\"","\"");
                gchar * SV = get_search_value (linebf, "server=\"","\"");
                gchar * FR = get_search_value (linebf, "farm=\"","\"");

                GlyrMemCache * cache = DL_init();
                cache->data = g_strdup_printf ("http://farm%s.static.flickr.com/%s/%s_%s.jpg",FR,SV,ID,SC);
                cache->size = strlen (cache->data);
                result_list = g_list_prepend (result_list,cache);

                g_free (ID);
                g_free (SC);
                g_free (SV);
                g_free (FR);

                g_free (linebf);
            }
        }
    }
    return result_list;
}

/////////////////////////////////

MetaDataSource photos_flickr_src =
{
    .name = "flickr",
    .key  = 'f',
    .parser    = photos_flickr_parse,
    .get_url   = photos_flickr_url,
    .type      = GLYR_GET_ARTIST_PHOTOS,
    .quality   = 40,
    .speed     = 65,
    .endmarker = NULL,
    .free_url  = true
};
