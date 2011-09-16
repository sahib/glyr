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

#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif


/* The Name of the SQL File */
#define GLYR_DB_FILENAME "metadata.db"

/**
* glyr_db_init:
* @root_path: Folder to create DB in
* 
* Allocates a new database object, and create a SQLite database 
* at the given path. The filename is in #GLYR_DB_FILENAME 
* You can use insert,delete and lookup on it.
*
* Returns: A newly allocated GlyrDatabase, free with glyr_db_destroy
*/
GlyrDatabase * glyr_db_init(char * root_path);

/** 
* glyr_db_destroy:
* @db_object: A database connection
*
* Close the connection and free all associated memory.
* You may not use it anymore.
*/
void glyr_db_destroy(GlyrDatabase * db_object);

/**
* glyr_db_lookup:
* @db: A database connection
* @query: Define what to search for
*
* The artist,album,title and type field are used to query
* the database. 
*
* If you used glyr_opt_lookup_db() to bind the DB to a query, 
* You may use glyr_get() as an alternative for this method.
* If your specify "local" in glyr_opt_from() only the DB is searched.
*
* Other query-fields honored by glyr_db_lookup:
* <itemizedlist>
* <listitem>
* <para>
* glyr_opt_download() - If false URLs to images are searched, true for real images.
* </para>
* </listitem>
* <listitem>
* <para>
* glyr_opt_from() - What provider the item came from.
* </para>
* </listitem>
* </itemizedlist>
*
* Returns: A newly allocated #GlyrMemCache or NULL if nothing found
*/
GlyrMemCache * glyr_db_lookup(GlyrDatabase * db, GlyrQuery * query);

/**
* glyr_db_insert:
* @db: A database connection
* @q: The query that was used to retrieve the @cache
* @cache: The cache to insert.
* 
* The cache wil be inserted to the db @db, @q is used to determine the artist, album, title and type.
*/
void glyr_db_insert(GlyrDatabase * db, GlyrQuery * q, GlyrMemCache * cache);

/**
* glyr_db_delete: 
* @db: The Database
* @query: Define what item shall be deleted.
* 
* The database is searched for the artist, album, title and type specified in @query.
* If items in the DB match they will deleted.
* 
* Other query-fields honored by glyr_db_delete:
* <itemizedlist>
* <listitem>
* <para>
* glyr_opt_download() - If false URLs to images are searched, true for real images.
* </para>
* </listitem>
* <listitem>
* <para>
* glyr_opt_from() - What provider the item came from.
* </para>
* </listitem>
* <listitem>
* <para>
* glyr_opt_number() - How many items to delete at a max.
* </para>
* </listitem>
* </itemizedlist>
* Returns: The number of deleted items.
*/
int glyr_db_delete(GlyrDatabase * db, GlyrQuery * query);

#ifdef __cplusplus
}
#endif

#endif 
