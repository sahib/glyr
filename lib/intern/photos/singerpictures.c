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

#define SPICS_BASE_URL "http://www.singerpictures.com/%s-pictures.html"

static const gchar * photos_singerlyrics_url(GlyrQuery * settings)
{
    gchar * result_url = NULL;
    gchar * space_to_min_artist = strreplace(settings->artist," ","-");
    if(space_to_min_artist != NULL)
    {
        gchar * prep_artist = prepare_string(space_to_min_artist,FALSE);
        if(prep_artist != NULL)
        {
            result_url = g_strdup_printf(SPICS_BASE_URL,prep_artist);
            g_free(prep_artist);
        }
        g_free(space_to_min_artist);
    }
    return result_url;
}

/*--------------------------------------------------------*/

#define SIZE_BEGIN "</a><br>"
#define SIZE_END   "<br>"
#define MAX_NUM_SIZE 16

static gboolean check_image_size(GlyrQuery * s, gchar * ref_to_img_start)
{
    if(s->img_min_size == -1 && s->img_max_size == -1)
    {
        return TRUE;
    }

    gboolean result = FALSE;
    gchar * size_begin = strstr(ref_to_img_start,SIZE_BEGIN);
    if(size_begin != NULL)
    {
        size_begin += (sizeof SIZE_BEGIN) - 1;
        gchar * size_end = strstr(size_begin,SIZE_END);
        if(size_end != NULL)
        {
            gchar size_buf[2][MAX_NUM_SIZE] = {{},{}};
            gint copy_to = 0, offset = 0;

            /* Parse the 'x_one X y_one' field */
            for(gint it = 0; &size_begin[it] != size_end && it < MAX_NUM_SIZE; it++)
            {
                if(size_begin[it] == 'X')
                {
                    copy_to = 1;
                    offset  = 0;
                }
                else
                {
                    size_buf[copy_to][offset++] = size_begin[it];
                }
            }

            gint x = g_ascii_strtoll(size_buf[0],NULL,10);
            gint y = g_ascii_strtoll(size_buf[0],NULL,10);
            gint ratio = (x+y)/2;

            if((s->img_min_size == -1 || ratio >= s->img_min_size) &&
               (s->img_max_size == -1 || ratio <= s->img_max_size))
                result = TRUE;
        }
    }
    return result;
}

/*--------------------------------------------------------*/

#define FINAL_BASE_URL "http://www.singerpictures.com/images/pic/%s.jpg"
#define URL_ID_START "rel=\"nofollow\"><img src='/images/pic/"
#define URL_ID_END   "_th."

static GList * photos_singerlyrics_parse(cb_object * capo)
{
    GList * result_list = NULL;
    gsize len = (sizeof URL_ID_START) - 1;
    gchar * url_id_start = capo->cache->data;

    while((continue_search(g_list_length(result_list),capo->s) && (url_id_start = strstr(url_id_start + len,URL_ID_START) ) != NULL))
    {
        if(check_image_size(capo->s, url_id_start))
        {
	    gchar * ID = get_search_value(url_id_start,URL_ID_START,URL_ID_END);
            if(ID != NULL)
            {
                GlyrMemCache * item = DL_init();
                item->data = g_strdup_printf(FINAL_BASE_URL,ID);
                item->size = strlen(item->data);

                result_list = g_list_prepend(result_list, item);
                g_free(ID);
            }
        }
    }
    return result_list;
}

/*--------------------------------------------------------*/

MetaDataSource photos_singerpictures_src =
{
    .name = "singerpictures",
    .key  = 's',
    .parser    = photos_singerlyrics_parse,
    .get_url   = photos_singerlyrics_url,
    .type      = GLYR_GET_ARTIST_PHOTOS,
    .quality   = 75,
    .speed     = 60,
    .endmarker = NULL,
    .free_url  = true
};
