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

#ifndef GLYR_COMMON_H
#define GLYR_COMMON_H

#include "../core.h"
#include "../stringlib.h"

/* Cover, photos */
const gchar * generic_amazon_url(GlyrQuery * sets, const gchar * response_group);
const gchar * generic_google_url(GlyrQuery * sets, const gchar * searchterm);
GList * generic_google_parse(cb_object * capo);
gboolean size_is_okay(int sZ, int min, int max);

/* Tags, relations */
GlyrMemCache * generic_musicbrainz_parse(cb_object * capo, gint * last_mbid, const gchar * include);
const gchar * generic_musicbrainz_url(GlyrQuery * sets);
gint please_what_type(GlyrQuery * s);

#endif
