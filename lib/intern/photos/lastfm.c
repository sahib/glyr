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

// No choice on format, because everything smaller is pratctically unusable
#define SIZE_FO "<size name=\"extralarge\""
#define API_KEY API_KEY_LASTFM

#define URL_BEGIN "\">"
#define URL_ENDIN "</size>"

const char * photos_lastfm_url(GlyrQuery * settings)
{
    return "http://ws.audioscrobbler.com/2.0/?method=artist.getimages&artist=%artist%&api_key="API_KEY;
}

GList * photos_lastfm_parse(cb_object * capo)
{
    char * root = capo->cache->data;
    GList * r_list = NULL;
    size_t urlc = 0;

    while ( (root = strstr(root,SIZE_FO)) != NULL && continue_search(urlc,capo->s))
    {
        char * begin = strstr(root,URL_BEGIN);
        if(begin)
        {
            begin += strlen(URL_BEGIN);
            char * endin = strstr(begin,URL_ENDIN);
            if(endin)
            {
                char * urlb = copy_value(begin,endin);
                if(urlb)
                {
                    GlyrMemCache * cache = DL_init();
                    cache->data = urlb;
                    r_list = g_list_prepend(r_list,cache);
                    urlc++;

                }
            }
        }
        root += strlen(SIZE_FO) - 1;
    }
    return r_list;
}

/*--------------------------------------------------------*/

MetaDataSource photos_lastfm_src =
{
    .name = "lastfm",
    .key  = 'l',
    .parser    = photos_lastfm_parse,
    .get_url   = photos_lastfm_url,
    .type      = GET_ARTIST_PHOTOS,
    .quality   = 90,
    .speed     = 80,
    .endmarker = NULL,
    .free_url  = false
};
