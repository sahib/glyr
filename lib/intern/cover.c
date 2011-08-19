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

#include "../core.h"
#include "../stringlib.h"
#include "generic.h"

/* ------------------------------------- */

static GList * factory(GlyrQuery * s, GList * list, gboolean * stop_me)
{
    GList * l =  generic_img_finalizer(s,list,stop_me,TYPE_COVER);
    return l;
}

/* ------------------------------------- */

bool vdt_cover(GlyrQuery * settings)
{
    if (settings && settings->artist && settings->album)
    {
        /* validate size */
        if(settings->cover.min_size <= 0)
            settings->cover.min_size = -1;

        if(settings->cover.max_size <= 0)
            settings->cover.max_size = -1;

        return true;
    }
    else
    {
        return false;
    }
}

/* ------------------------------------- */

/* PlugStruct */
MetaDataFetcher glyrFetcher_cover =
{
    .name = "cover",
    .type = GET_COVERART,
    .validate  = vdt_cover,
    .full_data = FALSE,
    .init    = NULL,
    .destroy = NULL,
    .finalize = factory,
    .default_parallel = 3
};

/* ------------------------------------- */
