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

const char * tracklist_musicbrainz_url(GlyQuery * sets)
{
    return "http://musicbrainz.org/ws/1/release/?type=xml&artist=%artist%&releasetypes=Official&limit=10&title=%album%&limit=1";
}

//const char * musicbrainz_get_id(
#define REL_ID_BEGIN "id=\""
#define REL_ID_ENDIN "\" ext:score="
#define REL_ID_FORM  "http://musicbrainz.org/ws/1/release/%s?type=xml&inc=tracks"

#define DUR_BEGIN "<duration>"
#define DUR_ENDIN "</duration>"
#define TIT_BEGIN "<title>"
#define TIT_ENDIN "</title>"

static GList * traverse_xml(const char * data, const char * url, cb_object * capo)
{
    char * beg = (char*)data;
    GList * collection = NULL;

    int ctr = 0;

    while(continue_search(ctr,capo->s) && (beg = strstr(beg+1,TIT_BEGIN)) != NULL)
    {
        char * dy;
        char * value = copy_value(beg+strlen(TIT_BEGIN),strstr(beg,TIT_ENDIN)); // 1 UP
        char * durat = copy_value(strstr(beg,DUR_BEGIN)+strlen(DUR_BEGIN),(dy = strstr(beg,DUR_ENDIN))); // 1 UP
        if(value && durat)
        {
            GlyMemCache * cont = DL_init();
            cont->data = beautify_lyrics(value);
            cont->size = strlen(cont->data);
            cont->duration = atoi(durat) / 1e3;
            cont->dsrc = strdup(url);
            collection = g_list_prepend(collection,cont);

            ctr++;

            // free & jump to next
            free(value);
            free(durat);
            beg = dy;
        }
    }
    return collection;
}

GList * tracklist_musicbrainz_parse(cb_object * capo)
{
    GList * ls = NULL;
    char * release_ID = NULL;
    if( (release_ID = copy_value(strstr(capo->cache->data,REL_ID_BEGIN)+strlen(REL_ID_BEGIN),strstr(capo->cache->data,REL_ID_ENDIN))) != NULL)
    {
        char * release_page_info_url = strdup_printf(REL_ID_FORM,release_ID);
        GlyMemCache * dlData = download_single(release_page_info_url,capo->s,NULL);
        if(dlData)
        {
            ls = traverse_xml(dlData->data,capo->url,capo);
            DL_free(dlData);
        }
        free(release_page_info_url);
        free(release_ID);
    }
    return ls;
}

/*--------------------------------------------------------*/

MetaDataSource tracklist_musicbrainz_src = {
	.name = "musicbrainz",
	.key  = 'm',
	.parser    = tracklist_musicbrainz_parse,
	.get_url   = tracklist_musicbrainz_url,
	.quality   = 90,
	.speed     = 90,
	.endmarker = NULL, 
	.free_url  = false,
	.type      = GET_TRACKLIST
};

