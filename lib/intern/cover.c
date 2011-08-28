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
    return generic_img_finalizer(s,list,stop_me,GLYR_TYPE_COVER);
}

/* ------------------------------------- */

bool vdt_cover(GlyrQuery * settings)
{
    if(settings && settings->artist && settings->album)
    {
		if(settings->artist[0] && settings->album[0])
		{
				/* validate size */
				if(settings->img_min_size <= 0)
					settings->img_min_size = -1;

				if(settings->img_max_size <= 0)
					settings->img_max_size = -1;

				return TRUE;
		}
    }
    glyr_message(2,settings,"Artist and Album is needed to retrieve coverart.\n");
    return FALSE;
}

/* ------------------------------------- */

/* PlugStruct */
MetaDataFetcher glyrFetcher_cover =
{
    .name = "cover",
    .type = GLYR_GET_COVERART,
    .validate  = vdt_cover,
    .full_data = FALSE,
    .init    = NULL,
    .destroy = NULL,
    .finalize = factory,
    .default_parallel = 3
};

/* ------------------------------------- */
