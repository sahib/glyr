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

#define INFO_BEGIN "<div id=\"content\">"
#define OPTN_BEGIN "Biography:<br />"
#define INFO_ENDIN "</div>"
/////////////////////////////////

static const gchar * ainfo_lyricsreg_url (GlyrQuery * s)
{
    return "http://www.lyricsreg.com/biography/${artist}/";
}

/////////////////////////////////

static GList * ainfo_lyricsreg_parse (cb_object * capo)
{
    GList * result_list = NULL;
    gchar * point_to_start = strstr (capo->cache->data,INFO_BEGIN);
    if (point_to_start != NULL)
    {
        gchar * opt_begin = strstr (point_to_start,OPTN_BEGIN);
        gsize skip_len = (sizeof INFO_BEGIN) - 1;
        if (opt_begin != NULL)
        {
            point_to_start = opt_begin;
            skip_len = (sizeof OPTN_BEGIN) - 1;
        }

        point_to_start += skip_len;
        gchar * end = strstr (point_to_start, INFO_ENDIN);
        if (end != NULL)
        {
            gsize info_len = end - point_to_start;
            if (info_len > 200)
            {
                gchar * info = copy_value (point_to_start, end);
                if (info != NULL)
                {
                    GlyrMemCache * result = DL_init();
                    result->data = info;
                    result->size = info_len;
                    result_list = g_list_prepend (result_list,result);
                }
            }
        }
    }
    return result_list;
}

/////////////////////////////////

MetaDataSource ainfo_lyricsreg_src =
{
    .name      = "lyricsreg",
    .key       = 'r',
    .free_url  = false,
    .type      = GLYR_GET_ARTIST_BIO,
    .parser    = ainfo_lyricsreg_parse,
    .get_url   = ainfo_lyricsreg_url,
    .quality   = 35,
    .speed     = 50,
    .endmarker = NULL
};
