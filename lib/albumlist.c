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
#include <stdbool.h>
#include <unistd.h>

#include "core.h"
#include "types.h"
#include "stringlib.h"

#include "albumlist/musicbrainz.h"

//-------------------------------------

// Add your's here
GlyPlugin albumlist_providers[] = {
        {"musicbrainz","m",  C_Y"music"C_"brainz",  false,  {albumlist_musicbrainz_parse, albumlist_musicbrainz_url, NULL, false}, GRP_SAFE | GRP_FAST},
        { NULL,        NULL, NULL,                  false,  {NULL,                        NULL,                      NULL, false}, GRP_NONE | GRP_NONE},
};

//-------------------------------------

GlyPlugin * glyr_get_albumlist_providers(void)
{
        return copy_table(albumlist_providers,sizeof(albumlist_providers));
}

//-------------------------------------

static GlyCacheList * albumlist_finalize(GlyCacheList * result, GlyQuery * settings)
{
        flag_lint(result, settings);
        return generic_finalizer(result,settings,TYPE_ALBUMLIST);
}

//-------------------------------------

GlyCacheList * get_albumlist(GlyQuery * settings)
{
        GlyCacheList * result = NULL;
        if(settings && settings->artist) {
                result = register_and_execute(settings, albumlist_finalize);
        } else {
                glyr_message(2,settings,stderr,C_R"* "C_"Artist is needed to retrieve a albumlist (o rly?).\n");
        }
        return result;
}
//-------------------------------------
