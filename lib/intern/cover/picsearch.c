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
#include "../common/picsearch.h"

/* ------------------------- */

static const gchar * cover_picsearch_url(GlyrQuery * s)
{
	return generic_picsearch_url(s,"${artist}+${album}+coverart");
}

/* ------------------------- */
static GList * cover_picsearch_parse(cb_object * capo)
{
	return generic_picsearch_parse(capo);
}

/* ------------------------- */

MetaDataSource cover_picsearch_src =
{
.name      = "picsearch",
.key       = 'p',
.parser    = cover_picsearch_parse,
.get_url   = cover_picsearch_url,
.type      = GLYR_GET_COVERART,
.quality   = 50,
.speed     = 60,
.endmarker = NULL,
.free_url  = true
};
