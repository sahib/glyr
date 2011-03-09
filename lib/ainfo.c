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

#include "ainfo/lastfm.h"
#include "ainfo/allmusic_com.h"

// Add your's here
GlyPlugin ainfo_providers[] =
{
//  full name       key  coloredname          use?   parser callback           geturl callback         free url?
    {"lastfm",      "l", "last"C_R"."C_"fm",  false,  {ainfo_lastfm_parse,     ainfo_lastfm_url,       NULL, true }, GRP_SAFE | GRP_FAST},
    {"allmusic",    "m", C_"all"C_C"music",   false,  {ainfo_allmusic_parse,   ainfo_allmusic_url,     NULL, false}, GRP_USFE | GRP_SLOW},
    { NULL,        NULL, NULL,                false, {NULL,                    NULL,                   NULL, false}, GRP_NONE | GRP_NONE},
};

GlyPlugin * glyr_get_ainfo_providers(void)
{
    return copy_table(ainfo_providers,sizeof(ainfo_providers));
}

static GlyCacheList * ainfo_finalize(GlyCacheList * result, GlyQuery * settings)
{
    if(!result) return NULL;

    size_t i = 0;
    GlyCacheList * r_list = DL_new_lst();

    for(i = 0; i < result->size; i++)
    {
        // call user defined callback
        if(settings->callback.download)
            settings->callback.download(result->list[i],settings);

	result->list[i]->type = TYPE_AINFO;
        DL_add_to_list(r_list,DL_copy(result->list[i]));
    }
    return r_list;
}

GlyCacheList * get_ainfo(GlyQuery * settings)
{
    GlyCacheList * result = NULL;
    if(settings && settings->artist)
    {
        result = register_and_execute(settings, ainfo_finalize);
    }
    else
    {
        glyr_message(2,settings,stderr,C_R":: "C_"Artist is needed to download artist description.\n");
    }
    return result;
}
