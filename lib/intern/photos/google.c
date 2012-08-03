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

#include "../../stringlib.h"
#include "../common/google.h"

/////////////////////////////////

static const gchar * photos_google_url (GlyrQuery * s)
{
    const gchar * result = NULL;
    gchar * searchterm = g_strdup ("${artist}+band+photos");
    if (searchterm != NULL)
    {
        result = generic_google_url (s,searchterm);
        g_free (searchterm);
    }
    return result;
}

/////////////////////////////////

static GList * photos_google_parse (cb_object * capo)
{
    return generic_google_parse (capo);
}

/////////////////////////////////

MetaDataSource photos_google_src =
{
    .name = "google",
    .key  = 'g',
    .parser    = photos_google_parse,
    .get_url   = photos_google_url,
    .type      = GLYR_GET_ARTIST_PHOTOS,
    .quality   = 10,
    .speed     = 90,
    .endmarker = NULL,
    .free_url  = true,
    .lang_aware = true
};
