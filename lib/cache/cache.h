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

#ifndef GLYR_CACHE_H
#define GLYR_CACHE_H

#include <sqlite3.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _GlyrDatabase {
	const char * root_path;
	sqlite3 * db_handle;
} GlyrDatabase;

GlyrDatabase * glyr_init_database(char * root_path);
void insert_data(GlyrDatabase * db, GlyrQuery * q, GlyrMemCache * cache);
void glyr_destroy_database(GlyrDatabase * db_object);

#ifdef __cplusplus
}
#endif

#endif 
