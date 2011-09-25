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
******************************************************/

#ifndef GLYR_CACHE_INTERN_H
#define GLYR_CACHE_INTERN_H

#include "types.h"
#include "core.h"
#include <glib.h>

/* Check if a file is contained in the db */
gboolean db_contains(GlyrDatabase * db, GlyrMemCache * cache);

/* Locksafe wrappers for sql functions */
gint sqlite3_blocking_step(sqlite3_stmt *pStmt);
gint sqlite3_blocking_prepare_v2(sqlite3 *db, const gchar *zSql, gint nSql, sqlite3_stmt **ppStmt, const char **pz);
gint sqlite3_blocking_exec(sqlite3 *, const gchar *, int (*callback)(void*,int,char**,char**), void *, gchar ** );

#endif
