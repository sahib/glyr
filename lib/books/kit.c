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
#include <stdlib.h>
#include <string.h>

#include "kit.h"

#include "../core.h"
#include "../core.h"
#include "../stringop.h"

#define RESULT "<td class=\"kvk-result-txt\">"
#define RESULT_LINK_START "href=\""
#define RESULT_LINK_END "\">"
#define RESULT_START ">"
#define RESULT_END "</a>"

const char * books_kit_url(GlyQuery * settings)
{
    return "http://kvk.ubka.uni-karlsruhe.de/hylib-bin/kvk/nph-kvk2.cgi?"
           "maske=kvk-last&"
           "lang=de&"
           "css=none&"
           "input-charset=utf-8&"
           "ALL=%artist%&"
           "Timeout=120&"
           "kataloge=SWB&"
           "kataloge=BVB&"
           "kataloge=NRW&"
           "kataloge=HEBIS&"
           "kataloge=HEBIS_RETRO&"
           "kataloge=KOBV_SOLR&"
           "kataloge=GBV&"
           "kataloge=DDB&"
           "kataloge=STABI_BERLIN&"
           "kataloge=TIB&"
           "kataloge=OEVK_GBV&"
           "kataloge=VD16&"
           "kataloge=VD17&"
           "kataloge=ZDB";
}

static GlyMemCache * get_field(char ** pointer, char * start, char * end, cb_object * capo)
{
    char * field = NULL;
    field = (field=getStr(pointer, start, end)) == NULL ? "" : field;

    if(field)
    {
        GlyMemCache * dl = DL_init();
        dl->data = strdup(field);
        dl->size = strlen(dl->data);
        dl->dsrc = strdup(capo->url);
        return dl;
    }
    return NULL;
}


GlyCacheList * books_kit_parse(cb_object * capo)
{
    char * pointer = capo->cache->data;
    GlyCacheList * ls = NULL;

    int urlc = 0;
    while ((pointer = strstr(pointer, RESULT)) != NULL && continue_search(urlc++,capo->s))
    {
        pointer+=strlen(RESULT);

        if(!ls) ls = DL_new_lst();

        DL_add_to_list(ls, get_field(&pointer, RESULT_LINK_START, RESULT_LINK_END,capo));
        DL_add_to_list(ls, get_field(&pointer, RESULT_START, RESULT_END,capo));
    }

    return ls;
}
