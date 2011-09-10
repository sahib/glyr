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
#include "../core.h"
#include "cache.h"
#include <glib.h>

/* ------------------------------------------------------------------ */
static void create_table_defs(GlyrDatabase * db);
static void execute(GlyrDatabase * db, const gchar * sql_statement);
/* ------------------------------------------------------------------ */

#define DB_FILE "metadata.sql"

GlyrDatabase * glyr_init_database(char * root_path)
{
	GlyrDatabase * to_return = NULL;
	if(root_path != NULL && g_file_test(root_path,G_FILE_TEST_IS_DIR | G_FILE_TEST_EXISTS) == TRUE)
	{
		sqlite3 * db_connection = NULL;
		gchar * db_file_path = g_strdup_printf("%s%s%s",root_path,(g_str_has_suffix(root_path,"/") ? "" : "/"),DB_FILE);
		gint db_err = sqlite3_open_v2(db_file_path,&db_connection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(db_err == SQLITE_OK)
		{
			to_return = g_malloc0(sizeof(GlyrDatabase));
			to_return->root_path = g_strdup(root_path);
			to_return->db_handle = db_connection;
			create_table_defs(to_return);
			puts("Succesfully established database connection");
		}
		else
		{
			glyr_message(-1,NULL,"Connecting to database failed: %s\n",sqlite3_errmsg(db_connection));
			sqlite3_close(db_connection);
		}
		g_free(db_file_path);
	}
	return to_return;
}

/* ------------------------------------------------------------------ */

void glyr_destroy_database(GlyrDatabase * db_object)
{
	if(db_object != NULL)
	{
		int db_err = sqlite3_close(db_object->db_handle);
		if(db_err == SQLITE_OK)
		{
			g_free((gchar*)db_object->root_path);
			g_free(db_object);
			puts("Closed database.");
		}
		else
		{
			glyr_message(-1,NULL,"Disconnecting databse failed: %s\n",sqlite3_errmsg(db_object->db_handle));
		}
	}
}

/* ------------------------------------------------------------------ */

static void execute(GlyrDatabase * db, const gchar * sql_statement)
{
	if(db && sql_statement)
	{
		puts(sql_statement);
		char * err_msg = NULL;
		sqlite3_exec(db->db_handle,sql_statement,NULL,NULL,&err_msg);
		if(err_msg != NULL)
		{
			fprintf(stderr, "SQL error: %s\n", err_msg);
			sqlite3_free(err_msg);
		}
	}
}

/* ------------------------------------------------------------------ */

static void create_table_defs(GlyrDatabase * db)
{
	//	printf(
	execute(db,
			"-- Provider\n"
			"create table if not exists providers (provider_name varchar(20) UNIQUE);\n"
			""
			"-- Artist\n"
			"create table if not exists artists (artist_name varchar(128) UNIQUE);"
			"create table if not exists albums (album_name varchar(128) UNIQUE);"
			"create table if not exists titles (title_name varchar(128) UNIQUE);"
			""
			"-- Enum\n"
			"create table if not exists image_types(image_type_name varchar(16) UNIQUE);"
			""
			"-- MetaData\n"
			"create table if not exists metadata("
			"			          artist_id integer,"
			"			          album_id integer,"
			"			          title_id integer,"
			"			          provider_id integer,"
			"			          source_url varchar(512),"
			"			          image_type_id integer,"
			"			          data_type integer,"
			"			          data_size integer,"
			"			          data_checksum blob,"
			"			          data blob"
			");"
			"create index if not exists index_artist_id   on metadata(artist_id);"
			"create index if not exists index_album_id    on metadata(album_id);"
			"create index if not exists index_title_id    on metadata(title_id);"
			"create index if not exists index_provider_id on metadata(provider_id);"
			"-- Insert imageformats\n"
			"insert or ignore into image_types values('jpeg');"
			"insert or ignore into image_types values('jpg');"
			"insert or ignore into image_types values('png');"
			"insert or ignore into image_types values('gif');"
			"insert or ignore into image_types values('tiff');"
			);
}

/* ------------------------------------------------------------------ */

static void insert_cache_data(GlyrDatabase * db, GlyrQuery * query, GlyrMemCache * cache)
{
	char * sql= sqlite3_mprintf("INSERT INTO metadata VALUES("
			"(select rowid from artists where artist_name = '%q'),"
			"(select rowid from albums  where album_name = '%q'),"
			"(select rowid from titles  where title_name = '%q'),"
			"(select rowid from providers where provider_name = '%q'),"
			"?,"
			"(select rowid from image_types where image_type_name = '%q'),"
			"?,?,?,?);",
			query->artist,
			query->album,
			query->title,
			cache->prov,
			cache->img_format
			);

	sqlite3_stmt *stmt = NULL;
	sqlite3_prepare_v2(db->db_handle, sql, strlen(sql) + 1, &stmt, NULL);

	sqlite3_bind_text(stmt, 1, cache->dsrc,strlen(cache->dsrc) + 1, SQLITE_TRANSIENT);
	sqlite3_bind_int( stmt, 2, query->type);
	sqlite3_bind_int( stmt, 3, cache->size);
	sqlite3_bind_blob(stmt, 4, cache->md5sum, sizeof cache->md5sum, SQLITE_TRANSIENT);
	sqlite3_bind_blob(stmt, 5, cache->data, cache->size, SQLITE_TRANSIENT);

	if(sqlite3_step(stmt) != SQLITE_DONE) 
	{
		fprintf(stderr,"Error message: %s\n", sqlite3_errmsg(db->db_handle));
	}
	sqlite3_finalize(stmt);

	sqlite3_free(sql);
}

#define INSERT_STRING(SQL,ARG) { gchar * sql = sqlite3_mprintf(SQL,ARG); execute(db,sql); sqlite3_free(sql); }


void insert_data(GlyrDatabase * db, GlyrQuery * q, GlyrMemCache * cache)
{
	if(db && q && cache)
	{
		execute(db,"BEGIN");
		INSERT_STRING("INSERT OR IGNORE INTO artists VALUES('%q');",q->artist);
		INSERT_STRING("INSERT OR IGNORE INTO albums  VALUES('%q');",q->album);
		INSERT_STRING("INSERT OR IGNORE INTO titles  VALUES('%q');",q->title);
		INSERT_STRING("INSERT OR IGNORE INTO providers VALUES('%q');",cache->prov);
		insert_cache_data(db,q,cache);
		execute(db,"COMMIT");
	}
}
