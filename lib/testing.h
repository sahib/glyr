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

#ifndef GLYR_TESTING_H 
#define GLYR_TESTING_H 

/**
 * SECTION:testing
 * @short_description: Testing interface to libglyr 
 * @title: Testing
 * @section_id:
 * @stability: Stable
 * @include: glyr/testing.h
 *
 * A bunch for function that are not meant for general use.
 * They are merely for developers that want to have automated 
 * tests for their providers.
 */
#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"

/**
 * glyr_testing_call_url:
 * @provider_name: Which provider to ask
 * @type: What type the provider belongs to
 * @query: What exactly to search for
 *
 * Call a provider's get_url() function
 * This is meant for testing purpose only.
 *
 * Returns: The URL that the provider would want to download
 **/
const char * glyr_testing_call_url(const char * provider_name, GLYR_GET_TYPE type, GlyrQuery * query);

/**
 * glyr_testing_call_parser:
 * @provider_name: Which provider to ask
 * @type: What type the provider belongs to
 * @query: What exactly to search for
 * @cache: Parseable Input to the parser (e.g. a HTML-page)
 *
 * Call a certain parser. Example: ("google",GLYR_GET_COVERART,&query,pagesource_cache); 
 * This is meant for testing purpose only.
 *
 * Returns: A list of more or less finished items. 
 **/
GlyrMemCache * glyr_testing_call_parser(const char * provider_name, GLYR_GET_TYPE type, GlyrQuery * query, GlyrMemCache * cache);


#ifdef __cplusplus
}
#endif

#endif
