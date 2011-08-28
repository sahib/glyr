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

//-------------------------------------

bool get_albumlist(GlyrQuery * settings)
{
    if(settings && settings->artist && settings->artist[0])
    {
        return true;
    }
    else
    {
        glyr_message(2,settings,"Artist is needed to retrieve a albumlist.\n");
        return false;
    }
}

//-------------------------------------

static GList * factory(GlyrQuery * s, GList * list, gboolean * stop_me)
{
    return generic_txt_finalizer(s,list,stop_me,GLYR_TYPE_ALBUMLIST);
}

//-------------------------------------

/* PlugStruct */
MetaDataFetcher glyrFetcher_albumlist =
{
    .name = "albumlist",
    .type = GLYR_GET_ALBUMLIST,
    .validate  = get_albumlist,
    .full_data = TRUE,
    .init    = NULL,
    .destroy = NULL,
    .finalize = factory
};
