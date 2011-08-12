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

#include "../../stringlib.h"
#include "../../core.h"

const char * albumlist_musicbrainz_url(GlyQuery * sets)
{
    return "http://musicbrainz.org/ws/1/release/?type=xml&artist=%artist%&releasetypes=\"Official\"";
}

#define ALBUM_BEGIN "<release type=\"Album Official\""
#define ALBUM_ENDIN "\" id=\""

#define TITLE_BEGIN "<title>"
#define TITLE_ENDIN "</title>"

GList * albumlist_musicbrainz_parse(cb_object * capo)
{
    GList * collection = NULL;
    int ctr = 0;
    char * node = capo->cache->data;
    while(continue_search(ctr,capo->s) && (node = strstr(node+1,ALBUM_BEGIN)) != NULL)
    {
        char * name = copy_value(strstr(node,TITLE_BEGIN) + strlen(TITLE_BEGIN),strstr(node,TITLE_ENDIN) + strlen(TITLE_ENDIN));
        if(name != NULL)
        {
            GlyMemCache * c = DL_init();
            c->data = beautify_lyrics(name);

            if(c->data != NULL)
            {
                c->size = strlen(c->data);
            }


            c->dsrc = strdup(capo->url);
            collection = g_list_prepend(collection,c);
            g_free(name);
            ctr++;
        }
    }
    return collection;
}

/*-----------------------------*/

MetaDataSource albumlist_musicbrainz_src =
{
    .name = "musicbrainz",
    .key = 'm',
    .free_url = false,
    .parser  = albumlist_musicbrainz_parse,
    .get_url = albumlist_musicbrainz_url,
    .type    = GET_ALBUMLIST,
    .quality = 95,
    .speed   = 95,
    .endmarker = NULL
};
