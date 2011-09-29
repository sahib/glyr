/***********************************************************
 * This file is part of glyr
 * + a commnandline tool and library to download various sort of musicrelated metadata.
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


#include "types.h"

/* Call a provider's get_url() function (for testing purpose only!)  */
const char * glyr_testing_call_url(const char * provider_name, GLYR_GET_TYPE type, GlyrQuery * query);

/* Call a certain parser; example: ("google",GLYR_GET_COVERART,&query,pagesource_cache); */
GlyrMemCache * glyr_testing_call_parser(const char * provider_name, GLYR_GET_TYPE type, GlyrQuery * query, GlyrMemCache * cache);
