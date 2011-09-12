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
#include "../../core.h"
#include "../../stringlib.h"
#include "../common.h"

/* ------------------------- */

const gchar * cover_google_url(GlyrQuery * s)
{
    const gchar * result = NULL;
    gchar * searchterm = g_strdup("${artist}+${album}+album");
    if(searchterm != NULL)
    {
        result = generic_google_url(s,searchterm);
        g_free(searchterm);
    }
    return result;
}

/* ------------------------- */

GList * cover_google_parse(cb_object * capo)
{
    return generic_google_parse(capo);
}

/* ------------------------- */

MetaDataSource cover_google_src =
{
    .name      = "google",
    .key       = 'g',
    .parser    = cover_google_parse,
    .get_url   = cover_google_url,
    .type      = GLYR_GET_COVERART,
    .quality   = 25,
    .speed     = 90,
    .endmarker = NULL,
    .free_url  = true
};
