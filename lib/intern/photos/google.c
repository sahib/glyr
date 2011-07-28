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

// Borrowing actual implementation from cover:google.c
#include "../cover/google.h"

#include "../../core.h"
#include "../../stringlib.h"

const char * photos_google_url(GlyQuery * s)
{
    const char * result = NULL;
    char * searchterm = strdup("%artist%+band+photos");
    if(searchterm != NULL)
    {
        result = generic_google_url(s,searchterm);
        free(searchterm);
    }
    return result;
}

GlyCacheList * photos_google_parse(cb_object * capo)
{
    return generic_google_parse(capo);
}

/*--------------------------------------------------------*/

MetaDataSource photos_google_src = {
	.name = "google",
	.key  = 'g',
	.parser    = photos_google_parse,
	.get_url   = photos_google_url,
	.type      = GET_ARTIST_PHOTOS,
	.endmarker = NULL,
	.free_url  = true
};
