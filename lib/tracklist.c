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

#include "types.h"
#include "core.h"
#include "stringlib.h"

#include "tracklist/musicbrainz.h"

// Add your's here
GlyPlugin tracklist_providers[] =
{
    {"musicbrainz","m",  C_Y"music"C_"brainz",  false,  {tracklist_musicbrainz_parse, tracklist_musicbrainz_url, NULL, false}, GRP_SAFE | GRP_FAST},
    { NULL,        NULL, NULL,                  false,  {NULL,                        NULL,                      NULL, false}, GRP_NONE | GRP_NONE},
};

GlyPlugin * glyr_get_tracklist_providers(void)
{
    return copy_table(tracklist_providers,sizeof(tracklist_providers));
}

static GlyCacheList * tracklist_finalize(GlyCacheList * result, GlyQuery * settings)
{
    return generic_finalizer(result,settings,TYPE_TRACK);
}

GlyCacheList * get_tracklist(GlyQuery * settings)
{
    GlyCacheList * result = NULL;
    if(settings && settings->artist && settings->album)
    {
        result = register_and_execute(settings, tracklist_finalize);
    }
    else
    {
        glyr_message(2,settings,stderr,C_R"* "C_"Artist and Album is needed to retrieve a tracklist (o rly?).\n");
    }
    return result;
}

/* PlugStruct */
MetaDataFetcher glyrFetcher_tracklist = {
	.name = "Tracklist Fetcher",
	.type = GET_TRACKLIST,
	.get  = get_tracklist,
	/* CTor | DTor */
	.init    = NULL,
	.destroy = NULL
};
