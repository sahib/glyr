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

/**
 * SECTION:cache
 * @short_description: A fast SQLite cache for glyr's results 
 * @title: Cache 
 * @section_id:
 * @stability: Stable
 * @include: glyr/cache.h
 *
 * The Cache offers application authors to store the items found 
 * by glyr persistently in a DB-Cache and get them again once needed.
 * Before usage it is necessary to open the DB via glyr_db_init(),
 * which expects a folder where the DB will be stored, after this
 * there are 4 operations you can use on the query:
 * <itemizedlist>
 * <listitem>
 * <para>
 * Lookup by a Query (glyr_db_lookup())
 * </para>
 * </listitem>
 * <listitem>
 * <para>
 * Delete by a Query (glyr_db_delete())
 * </para>
 * </listitem>
 * <listitem>
 * <para>
 * Insert (glyr_db_insert())
 * </para>
 * </listitem>
 * <listitem>
 * <para>
 * Iterate (glyr_db_foreach())
 * </para>
 * </listitem>
 * </itemizedlist>
 *
 *
 * If you want to insert "own" caches, e.g. to indicate that some artist wasn't found you could use 
 * the following piece of code:
 * <informalexample>
 * <programlisting>
        // Create a new "dummy" cache
        GlyrMemCache * ct = glyr_db_make_dummy();

        // Query with filled in artist, album, title, type,
        // and opened db
        glyr_db_insert(db,&q,ct);

        glyr_cache_free(ct);
 * </programlisting>
 * </informalexample>
 */

#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef int (*glyr_foreach_callback)(GlyrQuery * q, GlyrMemCache * item, void * userptr);

/* The Name of the SQL File */
#define GLYR_DB_FILENAME "metadata.db"

/**
* glyr_db_init:
* @root_path: Folder to create DB in
* 
* Allocates a new database object, and create a SQLite database 
* at the given path. The filename is in #GLYR_DB_FILENAME 
* You can now use insert,delete, edit and lookup on it.
*
* Returns: A newly allocated GlyrDatabase, free with glyr_db_destroy
*/
GlyrDatabase * glyr_db_init(const char * root_path);

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
* Other query-fields honored by glyr_db_lookup():
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
* glyr_opt_number() - How many items to return at max.
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
* Other query-fields honored by glyr_db_delete():
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

/**
* glyr_db_edit:
* @db: The Database
* @query: The query with set artist,album, type etc.
* @edited: The edited cache.
* 
* A simple convenience function to delete caches according to the settings specified in @query,
* (Same rules as in glyr_db_delete() apply here).
* After deleting the cache @edited in inserted. If @edited is a doubly linked list (next pointer is not NULL),
* then all items in the list are inserted.
* 
* You could have written it yourself like this:
* <informalexample>
* <programlisting>
* int glyr_db_edit(GlyrDatabase * db, GlyrQuery * query, GlyrMemCache * edited)
* {
*	int result = 0;
*	if(db && query)
*	{
*		result = glyr_db_delete(db,query);
*		if(result != 0)
*		{
*			for(GlyrMemCache * elem = edited; elem; elem = elem->next)
*			{
*				glyr_db_insert(db,query,edited);
*			}
*		}
*	}
*	return result;
* }
* </programlisting>
* </informalexample>
* 
* Returns: The number of replaced caches
*/
int glyr_db_edit(GlyrDatabase * db, GlyrQuery * query, GlyrMemCache * edited);

/**
* glyr_db_replace:
* @db: The Database
* @md5sum: The md5sum of the cache you want to edit.
* @query: The query with set artist,album, type etc.
* @data: The edited cache.
*
* Simple convenience function to edit caches in the Database.
* Best understood by example:
* <informalexample>
* <programlisting>
* // If you have a cache called 'c', that's already
* // In the Database: 
* // Save the old checksum, edit it, update the database.
* unsigned char old_md5sum[16] = {0};
* memcpy(old_md5sum,c->md5sum,16);
* glyr_cache_set_data(c,g_strdup("Changed the data - muahahah"),-1);
* c->rating = 4200;
* glyr_db_replace(s->local_db, old_md5sum, s, c);
* </programlisting>
* </informalexample>
*
* Some caveats:
* <itemizedlist>
* <listitem>
* <para>
* You may insert a cache several times, if the source url (cache->dsrc) is different,
* but with the same checksum. If you call glyr_db_replace() once more, the caches
* with the double md5sum get deleted and replaced by the new one.
* </para>
* </listitem>
* </itemizedlist>
*/
void glyr_db_replace(GlyrDatabase * db, unsigned char * md5sum, GlyrQuery * query, GlyrMemCache * data);


/**
* glyr_db_foreach:
* @db: A database connection
* @cb: The callback to call on each item.
* @userptr: A pointer to pass as second argument to the callback.
*
* Iterate over all items in the database.
* Callback may not be null. If callback returns a number != 0, 
* iteration aborts.
*
* Callback parameters:
*
* 1) The artist / album / title / type that was used to get this cache is stored in 'query', other fields are not filled.
*    
* 2) The actual cache completely filled.
*
* 3) The userpointer you passed as 3rd argument to glyr_db_foreach()
*
* This is useful if you want to have statistics or such.
*
*/
void glyr_db_foreach(GlyrDatabase * db, glyr_foreach_callback cb, void * userptr);


/**
 * glyr_db_make_dummy: 
 *
 * The idea behind this function is to create a dummy entry for the cache, e.g. to indicate a certain item
 * was not found. In this case you create a dummy with a 'rating' of -1 and push it into the DB. 
 * If one does a lookup (via glyr_get() or glyr_db_lookup()) then this cache will be returned, one is able
 * to check if the item was not found before. 
 *
 * Returns: A newly allocated cache, use glyr_cache_free() to free it.
 */
GlyrMemCache * glyr_db_make_dummy(void);

#ifdef __cplusplus
}
#endif

#endif 
