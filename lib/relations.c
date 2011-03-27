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

#include "relations/musicbrainz.h"

// Add your's here
GlyPlugin relations_providers[] =
{
    {"musicbrainz","m",  C_Y"music"C_"brainz",  false,  {relations_musicbrainz_parse,  relations_musicbrainz_url, NULL, true }, GRP_SAFE | GRP_FAST},
    { NULL,        NULL, NULL,                  false,  {NULL,                         NULL,                      NULL, false}, GRP_NONE | GRP_NONE},
};

GlyPlugin * glyr_get_relations_providers(void)
{
    return copy_table(relations_providers,sizeof(relations_providers));
}

static GlyCacheList * relations_finalize(GlyCacheList * result, GlyQuery * settings)
{
    flag_double_urls(result,settings);
    return generic_finalizer(result,settings,TYPE_RELATION);
}

GlyCacheList * get_relations(GlyQuery * settings)
{
    GlyCacheList * result = NULL;
    if(settings && settings->artist)
    {
        result = register_and_execute(settings, relations_finalize);
    }
    else
    {
        glyr_message(2,settings,stderr,C_R"* "C_"At least the artist is needed to get relations.\n");
    }
    return result;
}
