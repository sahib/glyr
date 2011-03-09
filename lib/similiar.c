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
#include "stringop.h"

#include "similiar/lastfm.h"

// Add your's here
GlyPlugin similiar_providers[] =
{
//  full name       key  coloredname          use?   parser callback           geturl callback         free url?
    {"lastfm",      "l", "last"C_R"."C_"fm",  false,  {similiar_lastfm_parse,  similiar_lastfm_url,  NULL, false}, GRP_SAFE | GRP_FAST},
    {NULL,          NULL, NULL,               false,  {NULL,                   NULL,                 NULL, false}, GRP_NONE | GRP_NONE}
};

GlyPlugin * glyr_get_similiar_providers(void)
{
    return copy_table(similiar_providers,sizeof(similiar_providers));
}

static GlyCacheList * similiar_finalize(GlyCacheList * result, GlyQuery * settings)
{
    if(!result) return NULL;

    size_t i = 0;
    GlyCacheList * r_list = DL_new_lst();

    for(i = 0; i < result->size; i++)
    {
        // call user defined callback
        if(settings->callback.download)
            settings->callback.download(result->list[i],settings);

	result->list[i]->type = TYPE_SIMILIAR;
        DL_add_to_list(r_list,DL_copy(result->list[i]));
    }
    return r_list;
}

GlyCacheList * get_similiar(GlyQuery * settings)
{
    GlyCacheList * result = NULL;
    if(settings && settings->artist)
    {
        result = register_and_execute(settings, similiar_finalize);
    }
    else
    {
        glyr_message(2,settings,stderr,C_R"* "C_"Artist is needed to find similiar artist (o rly?).\n");
    }
    return result;
}
