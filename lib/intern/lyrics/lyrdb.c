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

#define LYRDB_URL "http://webservices.lyrdb.com/lookup.php?q=%artist%|%title%&for=match&agent=mpdbox"

const char * lyrics_lyrdb_url(GlyrQuery * settings)
{
    return LYRDB_URL;
}

GList * lyrics_lyrdb_parse(cb_object * capo)
{
    GlyrMemCache * result = NULL;
    GList * r_list = NULL;

    char *slash;
    if( (slash = strchr(capo->cache->data,'\\')) )
    {
        char * uID = copy_value(capo->cache->data,slash);
        if(uID)
        {
            char * lyr_url = g_strdup_printf("http://webservices.lyrdb.com/getlyr.php?q=%s",uID);
            if(lyr_url)
            {
                GlyrMemCache * new_cache = download_single(lyr_url,capo->s,NULL);
                if(new_cache)
                {
                    char *buffer = g_malloc0(new_cache->size+1);

                    size_t i;
                    for(i=0; i < new_cache->size; i++)
                        buffer[i] = (new_cache->data[i] == '\r') ? ' ' : new_cache->data[i];

                    buffer[i] = 0;

                    result = DL_init();
                    result->data = buffer;
                    result->size = i;
                    result->dsrc = strdup(lyr_url);

                    DL_free(new_cache);
                }
                g_free(lyr_url);
            }
            g_free(uID);
        }
    }

    if(result)
    {
        r_list = g_list_prepend(r_list,result);
    }
    return r_list;
}

/*---------------------------------------------------*/

MetaDataSource lyrics_lyrdb_src =
{
    .name = "lyrdb",
    .key  = 'd',
    .parser    = lyrics_lyrdb_parse,
    .get_url   = lyrics_lyrdb_url,
    .type      = GET_LYRICS,
    .endmarker = NULL,
    .quality   = 75,
    .speed     = 50,
    .free_url  = false
};
