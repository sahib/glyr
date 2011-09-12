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

#include "../types.h"

#ifdef __cplusplus
extern "C"
{
#endif


/* The Name of the SQL File */
#define GLYR_DB_FILENAME "metadata.db"

GlyrDatabase * glyr_init_db(char * root_path);
void glyr_destroy_db(GlyrDatabase * db_object);

void glyr_db_insert(GlyrDatabase * db, GlyrQuery * q, GlyrMemCache * cache);
GlyrMemCache * glyr_db_lookup(GlyrDatabase * db, GlyrQuery * query);
bool glyr_db_contains(GlyrDatabase * db, GlyrMemCache * cache);
bool glyr_db_delete(GlyrDatabase * db, GlyrQuery * query);

#ifdef __cplusplus
}
#endif

#endif 
