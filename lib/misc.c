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

#include "stringlib.h"
#include "misc.h"

__attribute__((visibility("default")))
size_t glyr_levenshtein_strcmp(const char * string, const char * other)
{
    return levenshtein_strcmp(string,other);
}

__attribute__((visibility("default")))
size_t glyr_levenshtein_strnormcmp(const char * string, const char * other)
{
    return levenshtein_strnormcmp(NULL,string,other);
}
