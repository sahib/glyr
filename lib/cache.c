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

#include "cache_intern.h"
#include "cache.h"
#include "core.h"
#include "glyr.h"
#include "register_plugins.h"

///////////////////////////////

enum 
{
    SQL_TABLE_DEF,
    SQL_FOREACH,
    SQL_DELETE_SELECT,
    SQL_ACTUAL_DELETE,
    SQL_LOOKUP,
    SQL_INSERT_CACHE
};

static const char * sqlcode[] = 
{
    [SQL_TABLE_DEF] = 
        "PRAGMA synchronous = 1;                                                     \n"
        "PRAGMA temp_store = 2;                                                      \n"
        "BEGIN IMMEDIATE;                                                            \n"
        "-- Provider                                                                 \n"
        "CREATE TABLE IF NOT EXISTS providers (provider_name VARCHAR(20) UNIQUE);    \n"
        "                                                                            \n"
        "-- Artist                                                                   \n"
        "CREATE TABLE IF NOT EXISTS artists (artist_name VARCHAR(128) UNIQUE);       \n"
        "CREATE TABLE IF NOT EXISTS albums  (album_name  VARCHAR(128) UNIQUE);       \n"
        "CREATE TABLE IF NOT EXISTS titles  (title_name  VARCHAR(128) UNIQUE);       \n"
        "                                                                            \n"
        "-- Enum                                                                     \n"
        "CREATE TABLE IF NOT EXISTS image_types(image_type_name VARCHAR(16) UNIQUE); \n"
        "CREATE TABLE IF NOT EXISTS db_version(version INTEGER UNIQUE);              \n"
        "                                                                            \n"
        "-- MetaData                                                                 \n"
        "CREATE TABLE IF NOT EXISTS metadata(                                        \n"
        "                     artist_id INTEGER,                                     \n"
        "                     album_id  INTEGER,                                     \n"
        "                     title_id  INTEGER,                                     \n"
        "                     provider_id INTEGER,                                   \n"
        "                     source_url  VARCHAR(512),                              \n"
        "                     image_type_id INTEGER,                                 \n"
        "                     track_duration INTEGER,                                \n"
        "                     get_type INTEGER,                                      \n"
        "                     data_type INTEGER,                                     \n"
        "                     data_size INTEGER,                                     \n"
        "                     data_is_image INTEGER,                                 \n"
        "                     data_checksum BLOB,                                    \n"
        "                     data BLOB,                                             \n"
        "                     rating INTEGER,                                        \n"
        "                     timestamp FLOAT                                        \n"
        ");                                                                          \n"
        "CREATE INDEX IF NOT EXISTS index_artist_id   ON metadata(artist_id);        \n"
        "CREATE INDEX IF NOT EXISTS index_album_id    ON metadata(album_id);         \n"
        "CREATE INDEX IF NOT EXISTS index_title_id    ON metadata(title_id);         \n"
        "CREATE INDEX IF NOT EXISTS index_provider_id ON metadata(provider_id);      \n"
        "CREATE UNIQUE INDEX IF NOT EXISTS index_unique                              \n"
        "       ON metadata(get_type,data_type,data_checksum,source_url);            \n"
        "-- Insert imageformats                                                      \n"
        "INSERT OR IGNORE INTO image_types VALUES('jpeg');                           \n"
        "INSERT OR IGNORE INTO image_types VALUES('jpg');                            \n"
        "INSERT OR IGNORE INTO image_types VALUES('png');                            \n"
        "INSERT OR IGNORE INTO image_types VALUES('gif');                            \n"
        "INSERT OR IGNORE INTO image_types VALUES('tiff');                           \n"
        "INSERT OR IGNORE INTO db_version VALUES(2);                                 \n"
        "COMMIT;                                                                     \n",
    [SQL_FOREACH] = 
        "SELECT artist_name,                                      \n"
        "        album_name,                                      \n"
        "        title_name,                                      \n"
        "        provider_name,                                   \n"
        "        source_url,                                      \n"
        "        image_type_name,                                 \n"
        "        track_duration,                                  \n"
        "        get_type,                                        \n"
        "        data_type,                                       \n"
        "        data_size,                                       \n"
        "        data_is_image,                                   \n"
        "        data_checksum,                                   \n"
        "        data,                                            \n"
        "        rating,                                          \n"
        "        timestamp                                        \n"
        "FROM metadata as m                                       \n"
        "LEFT JOIN artists     AS a ON m.artist_id     = a.rowid  \n"
        "LEFT JOIN albums      AS b ON m.album_id      = b.rowid  \n"
        "LEFT JOIN titles      AS t ON m.title_id      = t.rowid  \n"
        "LEFT JOIN image_types AS i ON m.image_type_id = i.rowid  \n"
        "JOIN providers AS p on m.provider_id          = p.rowid  \n",
   [SQL_DELETE_SELECT] = 
        "SELECT get_type,                                     \n"
        "       artist_id,                                    \n"
        "       album_id,                                     \n"
        "       title_id,                                     \n"
        "       provider_id                                   \n"
        "       FROM metadata AS m                            \n"
        "LEFT JOIN artists    AS a ON a.rowid = m.artist_id   \n"
        "LEFT JOIN albums     AS b ON b.rowid = m.album_id    \n"
        "LEFT JOIN titles     AS t ON t.rowid = m.title_id    \n"
        "INNER JOIN providers AS p ON p.rowid = m.provider_id \n"
        "WHERE                                                \n"
        "       m.get_type  = %d                              \n"
        "   %s  -- Title  Contraint                           \n"
        "   %s  -- Album  Constraint                          \n"
        "   %s  -- Artist Constraint                          \n"
        "   AND p.provider_name IN(%s)                        \n"
        "   %s  -- 'IsALink' Constraint                       \n"
        "LIMIT %d;                                            \n",
   [SQL_ACTUAL_DELETE] = 
       "DELETE FROM metadata WHERE \n"
       "get_type    %s %s AND      \n"
       "artist_id   %s %s AND      \n"
       "album_id    %s %s AND      \n"
       "title_id    %s %s AND      \n"
       "provider_id %s %s;         \n",
   [SQL_LOOKUP] = 
       "SELECT artist_name,                                      \n"
       "        album_name,                                      \n"
       "        title_name,                                      \n"
       "        provider_name,                                   \n"
       "        source_url,                                      \n"
       "        image_type_name,                                 \n"
       "        track_duration,                                  \n"
       "        get_type,                                        \n"
       "        data_type,                                       \n"
       "        data_size,                                       \n"
       "        data_is_image,                                   \n"
       "        data_checksum,                                   \n"
       "        data,                                            \n"
       "        rating,                                          \n"
       "        timestamp                                        \n"
       "FROM metadata as m                                       \n"
       "LEFT JOIN artists AS a ON m.artist_id  = a.rowid         \n"
       "LEFT JOIN albums  AS b ON m.album_id   = b.rowid         \n"
       "LEFT JOIN titles  AS t ON m.title_id   = t.rowid         \n"
       "JOIN providers as p on m.provider_id   = p.rowid         \n"
       "LEFT JOIN image_types as i on m.image_type_id = i.rowid  \n"
       "WHERE m.get_type = %d                                    \n"
       "                   %s  -- Title constr.                  \n"
       "                   %s  -- Album constr.                  \n"
       "                   %s  -- Artist constr.                 \n"
       "                   %s                                    \n"
       "           AND provider_name IN(%s)                      \n"
       "LIMIT %d;                                                \n",
   [SQL_INSERT_CACHE] = 
       "INSERT OR IGNORE INTO metadata VALUES(                                \n"
       "  (SELECT rowid FROM artists   WHERE artist_name   = LOWER('%q')),    \n"
       "  (SELECT rowid FROM albums    WHERE album_name    = LOWER('%q')),    \n"
       "  (SELECT rowid FROM titles    WHERE title_name    = LOWER('%q')),    \n"
       "  (SELECT rowid FROM providers WHERE provider_name = LOWER('%q')),    \n"
       "  ?,                                                                  \n"
       "  (SELECT rowid FROM image_types WHERE image_type_name = LOWER('%q')),\n"
       "  ?,?,?,?,?,?,?,?,?                                                   \n"
       ");                                                                    \n"
};

////////////////////////////////////////////////////////
////////////////////// Prototypes //////////////////////
////////////////////////////////////////////////////////

static void insert_cache_data(GlyrDatabase * db, GlyrQuery * query, GlyrMemCache * cache);
static void execute(GlyrDatabase * db, const gchar * sql_statement);
static gchar * convert_from_option_to_sql(GlyrQuery * q);

static double get_current_time(void);
static void add_to_cache_list(GlyrMemCache ** list, GlyrMemCache * to_add);

static int delete_callback(void * result, int argc, char ** argv, char ** azColName);
static int select_callback(void * result, int argc, char ** argv, char ** azColName);


////////////////////////////////////////////////////////
////////////////// Useful Defines //////////////////////
////////////////////////////////////////////////////////

#define INSERT_STRING(SQL,ARG) {                        \
    if(SQL && ARG) {                                    \
    /* We have to use _ascii_ here,                   */\
    /* since there seems to be some encoding problems */\
    /* in SQLite, which are triggered by comparing    */\
    /* lower and highercase umlauts for example       */\
    /* Simple encoding-indepent lowercase prevents it */\
        gchar * lower_str = g_ascii_strdown(ARG,-1);    \
        gchar * sql = sqlite3_mprintf(SQL,lower_str);   \
        execute(db,sql);                                \
        sqlite3_free(sql);                              \
        g_free(lower_str);                              \
    }                                                   \
}

////////////////////////////////////////////////////////

/* Ensure no invalid data comes in */
#define ABORT_ON_FAILED_REQS(REQS,OPT_ARG,ARG) {                   \
    if((REQS & OPT_ARG) == 0 && ARG == NULL) {                     \
        glyr_message(-1,NULL,"Warning: %s != NULL failed",#ARG);   \
        goto rollback;                                             \
    }                                                              \
}

////////////////////////////////////////////////////////

#define ADD_CONSTRAINT(TO_CONSTR, FIELDNAME, VARNAME)                    \
{                                                                        \
    /* We have to use _ascii_ here,                   */                 \
    /* since there seems to be some encoding problems */                 \
    /* in SQLite, which are triggered by comparing    */                 \
    /* lower and highercase umlauts for example       */                 \
    /* Simple encoding-indepent lowercase prevents it */                 \
                                                                         \
    gchar * lower = g_ascii_strdown(VARNAME,-1);                         \
    if(lower != NULL)                                                    \
    {                                                                    \
        TO_CONSTR = sqlite3_mprintf("AND %s = '%q'\n",FIELDNAME,lower);  \
        g_free(lower);                                                   \
    }                                                                    \
}

////////////////////////////////////////////////////////

#define CACHE_GET_PROVIDER(cache) (((cache)&&(cache->prov)) ? ((cache)->prov) : "none")

////////////////////////////////////////////////////////

#define SQL_BIND_TEXT(stmt,text,pos) {                                               \
    int cpPos = pos;                                                                 \
    if(stmt && text) {                                                               \
        int rc = sqlite3_bind_text(stmt,cpPos,text,strlen(text) + 1, SQLITE_STATIC); \
        if(rc != SQLITE_OK) {                                                        \
            printf("Could not bind value: %d\n",rc);                                 \
        }                                                                            \
    }                                                                                \
}                                                                                    \

////////////////////////////////////////////////////////

/* How long to wait till returning SQLITE_BUSY */
#define DB_BUSY_WAIT 5000

#define DO_PROFILE false 

#if DO_PROFILE
static GTimer * select_callback_timer = NULL;
static float select_callback_spent = 0;
#endif

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

typedef struct
{
    GlyrDatabase * con;
    gint deleted;
    gint max_delete;

} delete_callback_data;


typedef struct
{
    GlyrMemCache ** result;
    GlyrQuery * query;
    gint counter;
    glyr_foreach_callback cb;
    void * userptr;

} select_callback_data;

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

GlyrDatabase * glyr_db_init(const char * root_path)
{
#if DO_PROFILE 
    GTimer * open_db = g_timer_new();
    select_callback_timer = g_timer_new();
#endif
    if(sqlite3_threadsafe() == FALSE)
    {
        glyr_message(-1,NULL,"WARNING: Your SQLite version seems not to be threadsafe? \n"
                "         Expect corrupted data and other weird behaviour!\n");
    }

    GlyrDatabase * to_return = NULL;
    if(root_path != NULL && g_file_test(root_path,G_FILE_TEST_IS_DIR | G_FILE_TEST_EXISTS) == TRUE)
    {
        sqlite3 * db_connection = NULL;

        /* Use file:// Urls when supported */
#if SQLITE_VERSION_NUMBER >= 3007007
        gchar * db_file_path = g_strdup_printf("file://%s%s%s",root_path,(g_str_has_suffix(root_path,"/") ? "" : "/"),GLYR_DB_FILENAME);
        gint db_err = sqlite3_open_v2(db_file_path,&db_connection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI | SQLITE_OPEN_FULLMUTEX, NULL);
#else
        gchar * db_file_path = g_strdup_printf("%s%s%s",root_path,(g_str_has_suffix(root_path,"/") ? "" : "/"),GLYR_DB_FILENAME);
        gint db_err = sqlite3_open_v2(db_file_path,&db_connection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL);
#endif

        if(db_err == SQLITE_OK)
        {
            to_return = g_malloc0(sizeof(GlyrDatabase));
            to_return->root_path = g_strdup(root_path);
            to_return->db_handle = db_connection;
            sqlite3_busy_timeout(db_connection,DB_BUSY_WAIT);

            /* Now create the Tables via sql */
            execute(to_return,(char*)sqlcode[SQL_TABLE_DEF]);
        }
        else
        {
            glyr_message(-1,NULL,"Connecting to database failed: %s\n",sqlite3_errmsg(db_connection));
            sqlite3_close(db_connection);
        }
        g_free(db_file_path);
    }
#if DO_PROFILE
    g_message("Time to open DB: %lf\n",g_timer_elapsed(open_db,NULL));
    g_timer_destroy(open_db);
#endif
    return to_return;
}

////////////////////////////////////
////////////////////////////////////
////////////////////////////////////


void glyr_db_destroy(GlyrDatabase * db_object)
{
    if(db_object != NULL)
    {
        int db_err = sqlite3_close(db_object->db_handle);
        if(db_err == SQLITE_OK)
        {
            g_free((gchar*)db_object->root_path);
            g_free(db_object);
        }
        else
        {
            glyr_message(-1,NULL,"Disconnecting database failed: %s\n",sqlite3_errmsg(db_object->db_handle));
        }
    }
}

////////////////////////////////////
////////////////////////////////////
////////////////////////////////////

int glyr_db_edit(GlyrDatabase * db, GlyrQuery * query, GlyrMemCache * edited)
{
    int result = 0;
    if(db && query)
    {
        result = glyr_db_delete(db,query);
        if(result != 0)
        {
            for(GlyrMemCache * elem = edited; elem; elem = elem->next)
            {
                glyr_db_insert(db,query,edited);
            }
        }
    }
    return result;
}

////////////////////////////////////
////////////////////////////////////
////////////////////////////////////

void glyr_db_replace(GlyrDatabase * db, unsigned char * md5sum, GlyrQuery * query, GlyrMemCache * data)
{
    if(db != NULL && md5sum != NULL) 
    {
        gchar * sql = "DELETE FROM metadata WHERE data_checksum = ? ;\n";
        sqlite3_stmt *stmt = NULL;
        sqlite3_prepare_v2(db->db_handle, sql, strlen(sql) + 1, &stmt, NULL);
        sqlite3_bind_blob(stmt, 1, md5sum, 16, SQLITE_STATIC);

        if(sqlite3_step(stmt) != SQLITE_DONE) 
        {
            glyr_message(1,query,"Error message: %s\n", sqlite3_errmsg(db->db_handle));
        }

        sqlite3_finalize(stmt);

        if(data != NULL)
        {
            glyr_db_insert(db,query,data);
        }
    }
}

////////////////////////////////////
////////////////////////////////////
////////////////////////////////////


gint glyr_db_delete(GlyrDatabase * db, GlyrQuery * query)
{
    gint result = 0;
    if(db && query)
    {
        /* Find out which fields are required for this getter */
        GLYR_FIELD_REQUIREMENT reqs = glyr_get_requirements(query->type);

        /* Spaces in SQL statements just for pretty debug printing */
        gchar * artist_constr = "";
        if((reqs & GLYR_REQUIRES_ARTIST) != 0 && query->artist)
        {
            ADD_CONSTRAINT(artist_constr,"a.artist_name",query->artist);
        }

        gchar * album_constr  = "";
        if((reqs & GLYR_REQUIRES_ALBUM ) != 0 && query->album)
        {                  
            ADD_CONSTRAINT(album_constr,"b.album_name",query->album);
        }

        gchar * title_constr = "";
        if((reqs & GLYR_REQUIRES_TITLE ) != 0 && query->title)
        {
            ADD_CONSTRAINT(title_constr,"t.title_name",query->title);
        }

        /* Get a SQL formatted list of enabled providers: IN('lastfm','...') */
        gchar * from_argument_list = convert_from_option_to_sql(query);

        /* Check if links should be deleted */        
        gchar * img_url_constr = "";
        if(TYPE_IS_IMAGE(query->type))
        {
            if(query->download == FALSE)
            {
                img_url_constr = sqlite3_mprintf("AND data_type = %d ", GLYR_TYPE_IMG_URL);
            }
            else
            {
                img_url_constr = sqlite3_mprintf("AND NOT data_type = %d ", GLYR_TYPE_IMG_URL);
            }
        }

        gchar * sql = sqlite3_mprintf(sqlcode[SQL_DELETE_SELECT],
                query->type,        /* Limit to <type>             */
                title_constr,       /* Title Constr, may be empty  */ 
                album_constr,       /* Album Constr, may be empty  */ 
                artist_constr,      /* Artist Constr, may be empty */
                from_argument_list, /* Provider contraint          */ 
                img_url_constr,     /* Search for links?           */
                query->number       /* LIMIT to <number>           */
                );


        if(sql != NULL)
        {
            delete_callback_data cb_data;
            cb_data.con = db;
            cb_data.deleted = 0;
            cb_data.max_delete = query->number;

            gchar * err_msg = NULL;
            sqlite3_exec(db->db_handle,sql,delete_callback,&cb_data,&err_msg);
            if(err_msg != NULL)
            {
                glyr_message(-1,NULL,"SQL Delete error: %s\n",err_msg);
                sqlite3_free(err_msg);
            }
            sqlite3_free(sql);
            result = cb_data.deleted;
        }

        /**
         * Free ressources with according free calls,
         */

        if(*artist_constr)
            sqlite3_free(artist_constr);

        if(*album_constr)
            sqlite3_free(album_constr);

        if(*title_constr)
            sqlite3_free(title_constr);

        if(*img_url_constr)
            sqlite3_free(img_url_constr);

        g_free(from_argument_list);

    }
    return result;
}

////////////////////////////////////
////////////////////////////////////
////////////////////////////////////


void glyr_db_foreach(GlyrDatabase * db, glyr_foreach_callback cb, void * userptr)
{
    if(db != NULL && cb != NULL)
    {
        select_callback_data scb_data;
        scb_data.cb = cb;
        scb_data.userptr = userptr;

        GlyrQuery dummy;
        dummy.number = G_MAXINT;
        scb_data.query = &dummy;
        scb_data.counter = 0;
        scb_data.result = NULL;

        int rc = SQLITE_OK;
        char * err_msg = NULL;
        if((rc = sqlite3_exec(db->db_handle,sqlcode[SQL_FOREACH],select_callback,&scb_data,&err_msg)) != SQLITE_OK)
        {
            if(rc != SQLITE_ABORT)
            {
                glyr_message(-1,NULL,"SQL Foreach error: %s\n",err_msg);
            }
            sqlite3_free(err_msg);
        }
    }
}

////////////////////////////////////
////////////////////////////////////
////////////////////////////////////

GlyrMemCache * glyr_db_lookup(GlyrDatabase * db, GlyrQuery * query)
{
    GlyrMemCache * result = NULL;
    if(db != NULL && query != NULL)
    {
        GLYR_FIELD_REQUIREMENT reqs = glyr_get_requirements(query->type);
        gchar * artist_constr = "";
        if((reqs & GLYR_REQUIRES_ARTIST) != 0)
        {
            ADD_CONSTRAINT(artist_constr,"artist_name",query->artist);
        }
        gchar * album_constr  = "";
        if((reqs & GLYR_REQUIRES_ALBUM ) != 0)
        {
            ADD_CONSTRAINT(album_constr,"album_name",query->album);
        }

        gchar * title_constr = "";
        if((reqs & GLYR_REQUIRES_TITLE ) != 0)
        {
            ADD_CONSTRAINT(title_constr,"title_name",query->title);
        }

        gchar * from_argument_list = convert_from_option_to_sql(query);
        gchar * img_url_constr = "";

        if(TYPE_IS_IMAGE(query->type))
        {
            if(query->download == FALSE)
            {
                img_url_constr = sqlite3_mprintf("AND data_type = %d ", GLYR_TYPE_IMG_URL);
            }
            else
            {
                img_url_constr = sqlite3_mprintf("AND NOT data_type = %d ", GLYR_TYPE_IMG_URL);
            }
        }

        gchar * sql = sqlite3_mprintf(sqlcode[SQL_LOOKUP],
                query->type,
                title_constr,
                album_constr,
                artist_constr,
                img_url_constr,
                from_argument_list, 
                query->number
                );


        if(sql != NULL)
        {
            select_callback_data data;
            data.result = &result;
            data.query = query;
            data.counter = 0;
            data.cb = NULL;
            data.userptr = NULL;

            gchar * err_msg = NULL;
            sqlite3_exec(db->db_handle,sql,select_callback,&data,&err_msg);
            if(err_msg != NULL)
            {
                glyr_message(-1,NULL,"glyr_db_lookup: %s\n",err_msg);
                sqlite3_free(err_msg);
            }
            sqlite3_free(sql);
        }

#if DO_PROFILE
        g_message("Spent %.5f Seconds in Selectcallback.\n",select_callback_spent);
        select_callback_spent = 0;
#endif

        if(*artist_constr)
        {
            sqlite3_free(artist_constr);
        }
        if(*album_constr)
        {
            sqlite3_free(album_constr);
        }
        if(*title_constr)
        {
            sqlite3_free(title_constr);
        }

        g_free(from_argument_list);

        if(*img_url_constr != '\0')
        {
            sqlite3_free(img_url_constr);
        }
    }
    return result;
}

////////////////////////////////////
////////////////////////////////////
////////////////////////////////////


void glyr_db_insert(GlyrDatabase * db, GlyrQuery * q, GlyrMemCache * cache)
{
    if(db && q && cache)
    {
        GLYR_FIELD_REQUIREMENT reqs = glyr_get_requirements(q->type);
        execute(db,"BEGIN IMMEDIATE;");
        if((reqs & GLYR_REQUIRES_ARTIST) || (reqs & GLYR_OPTIONAL_ARTIST)) {
            ABORT_ON_FAILED_REQS(reqs,GLYR_OPTIONAL_ARTIST,q->artist); 
            INSERT_STRING("INSERT OR IGNORE INTO artists VALUES('%q');",q->artist); 
        }
        if((reqs & GLYR_REQUIRES_ALBUM) || (reqs & GLYR_OPTIONAL_ALBUM)) {
            ABORT_ON_FAILED_REQS(reqs,GLYR_OPTIONAL_ALBUM,q->album); 
            INSERT_STRING("INSERT OR IGNORE INTO albums  VALUES('%q');",q->album);
        }
        if((reqs & GLYR_REQUIRES_TITLE) || (reqs & GLYR_OPTIONAL_TITLE)) {
            ABORT_ON_FAILED_REQS(reqs,GLYR_OPTIONAL_TITLE,q->title); 
            INSERT_STRING("INSERT OR IGNORE INTO titles  VALUES('%q');",q->title);
        }

        gchar * provider = CACHE_GET_PROVIDER(cache);
        INSERT_STRING("INSERT OR IGNORE INTO providers VALUES('%q');",provider);
        insert_cache_data(db,q,cache);

rollback:
        execute(db,"COMMIT;");
    }
}

////////////////////////////////////
////////////////////////////////////
////////////////////////////////////

GlyrMemCache * glyr_db_make_dummy(void)
{
    GlyrMemCache * c = glyr_cache_new();
    glyr_cache_set_data(c,g_strdup("[dummy]"),-1);
    c->rating = -1;
    return c;
}

////////////////////////////////////
////////////////////////////////////
////////////////////////////////////

// --------- INTERNALS ------------ //

static void execute(GlyrDatabase * db, const gchar * sql_statement)
{
    if(db && sql_statement)
    {
        char * err_msg = NULL;
        sqlite3_exec(db->db_handle,sql_statement,NULL,NULL,&err_msg);
        if(err_msg != NULL)
        {
            glyr_message(-1,NULL,"glyr_db_execute: SQL error: %s\n", err_msg);
            sqlite3_free(err_msg);
        }
    }
}

////////////////////////////////////
////////////////////////////////////
////////////////////////////////////

/**
 *  Return the current time as double:
 *  <seconds>.<microseconds/one_second>
 */
static double get_current_time(void)
{
    struct timeval tim;
    gettimeofday(&tim, NULL);
    return (double)tim.tv_sec + ((double)tim.tv_usec/1e6);
}

////////////////////////////////////

static void insert_cache_data(GlyrDatabase * db, GlyrQuery * query, GlyrMemCache * cache)
{
    if(db && query && cache)
    {
        int pos = 1;
        char * sql = sqlite3_mprintf(sqlcode[SQL_INSERT_CACHE],
                query->artist,
                query->album,
                query->title,
                CACHE_GET_PROVIDER(cache),
                cache->img_format
                );

        sqlite3_stmt *stmt = NULL;
        sqlite3_prepare_v2(db->db_handle, sql, strlen(sql) + 1, &stmt, NULL);

        SQL_BIND_TEXT(stmt,cache->dsrc,pos++);
        sqlite3_bind_int(stmt, pos++, cache->duration);
        sqlite3_bind_int(stmt, pos++, query->type);
        sqlite3_bind_int(stmt, pos++, cache->type);
        sqlite3_bind_int(stmt, pos++, cache->size);
        sqlite3_bind_int(stmt, pos++, cache->is_image);
        sqlite3_bind_blob(stmt,pos++, cache->md5sum, sizeof cache->md5sum, SQLITE_STATIC);

        if(cache->data != NULL) {
            sqlite3_bind_blob(stmt, pos++, cache->data, cache->size, SQLITE_STATIC);
        } else {
            glyr_message(1,query,"glyr: Warning: Attempting to insert cache with missing data!\n");
        }

        sqlite3_bind_int(stmt, pos++, cache->rating);
        sqlite3_bind_double(stmt,pos++, get_current_time());

        if(sqlite3_step(stmt) != SQLITE_DONE) {
            glyr_message(1,query,"glyr_db_insert: SQL failure: %s\n", sqlite3_errmsg(db->db_handle));
        }

        sqlite3_finalize(stmt);
        sqlite3_free(sql);
    }
}



////////////////////////////////////
////////////////////////////////////
////////////////////////////////////

#define DEBUG_LIST false 

static void add_to_cache_list(GlyrMemCache ** list, GlyrMemCache * to_add)
{
    if(to_add && list)
    {
        GlyrMemCache * head = *list;
        if(head == NULL)
        {
            /* Initialize list */
            *list = to_add;
        }
        else 
        {
            /* Find rating in list
             * head will store the first non-matchin item,
             * tail = head->prev
             * */
            GlyrMemCache * tail = head;
            while(head && head->rating > to_add->rating)
            {
                tail = head;
                head = head->next;
            }


            /* Now see what timestamp we have,
             * and sort it accordingly, younger 
             * caches (== higher timestamp), are sorted
             * at the start 
             */
            if(head != NULL)
            {
                int last_rating = head->rating;
                while(head && head->rating == last_rating && head->timestamp > to_add->timestamp)
                {
                    GlyrMemCache * p = head->next;
                    if(p && p->rating == last_rating)
                    {
                        tail = head;
                        head = p;
                    }
                    else if(p == NULL)
                    {
                        tail = head;
                        head = p;
                    }
                    else break;

                }
            }

            /* Check if we're at the end of the list,
             * If so just append it, 
             * else we insert to_add before head
             */ 
            g_assert(tail);
            if(head != NULL)
            {
                GlyrMemCache * prev = head->prev;
                if(prev != NULL)
                    prev->next = to_add;

                to_add->prev = prev;
                to_add->next = head;

                head->prev = to_add;

                if(prev == NULL)
                    *list = to_add;
            }
            else /* We're at the end */
            {
                tail->next = to_add;
                to_add->prev = tail; 
            }
        }
    }

#if DEBUG_LIST
    GlyrMemCache * p = *list;
    while(p != NULL)
    {
        char * surr = (to_add == p) ? "*" : "|";
        g_printerr("%s(%d|%2.10f)%s %c ",surr,p->rating,p->timestamp,surr,(p->next) ? '-' : ' ');
        p = p->next;
    }
    puts("");
#endif

}

////////////////////////////////////
////////////////////////////////////
////////////////////////////////////

/* Convert a single result from the DB to an actual Cache */
static int select_callback(void * result, int argc, char ** argv, char ** azColName)
{
#if DO_PROFILE
    g_timer_start(select_callback_timer);
#endif


    int rc = 0;
    select_callback_data * data = result;
    GlyrMemCache ** list = data->result;

    if(argc >= 15 && data->counter < data->query->number)
    {
        GlyrMemCache * cache = DL_init();
        if(cache != NULL)
        {
            cache->prov = g_strdup(argv[3]);
            cache->dsrc = g_strdup(argv[4]);
            cache->img_format = g_strdup(argv[5]);

            cache->duration = (argv[6]  ? strtol(argv[6],NULL,10)   : 0);
            cache->type =     (argv[8]  ? strtol(argv[8],NULL,10)   : 0);
            cache->size =     (argv[9]  ? strtol(argv[9],NULL,10)   : 0);
            cache->is_image = (argv[10] ? strtol(argv[10],NULL,10) : 0);

            if(argv[11] != NULL)
            {
                memcpy(cache->md5sum,argv[11],16);
            }

            if(argv[12] != NULL && cache->size > 0)
            {
                cache->data = g_malloc0(cache->size + 1);
                memcpy(cache->data,argv[12],cache->size);
                cache->data[cache->size] = 0;
            }

            cache->rating = (argv[13] ? strtol(argv[13],NULL,10) : 0);

            /* Timestamp */
            if(argv[14] != NULL)
            {
                /* Normal strtod() cuts off part behin the comma.. */
                cache->timestamp = (argv[14] ? g_ascii_strtod(argv[14],NULL) : 0);
            }

            /* We're in the cache, so this one was cached.. :) */
            cache->cached = TRUE;

            if(list != NULL)
            {
                add_to_cache_list(list,cache);
            }
            else if(data->cb != NULL && cache)
            {
                GlyrQuery q;
                glyr_query_init(&q);

                if(argv[7] != NULL)
                {
                    GLYR_GET_TYPE type = strtol(argv[7],NULL,10);
                    glyr_opt_type(&q,type);
                }

                glyr_opt_artist(&q,argv[0]);
                glyr_opt_album(&q, argv[1]);
                glyr_opt_title(&q, argv[2]);

                rc = data->cb(&q,cache,data->userptr);

                glyr_query_destroy(&q);
                DL_free(cache);
            }
        }
    }

    data->counter++;
#if DO_PROFILE
    g_timer_stop(select_callback_timer);
    select_callback_spent += g_timer_elapsed(select_callback_timer,NULL);
#endif
    return rc;
}

////////////////////////////////////
////////////////////////////////////
////////////////////////////////////

static gchar * convert_from_option_to_sql(GlyrQuery * q)
{
    gchar * result = g_strdup("'none'");

    for(GList * elem = r_getSList(); elem; elem = elem->next)
    {
        MetaDataSource * item = elem->data;
        if(item && (q->type == item->type || item->type == GLYR_GET_ANY))
        {
            if(provider_is_enabled(q,item) == TRUE)
            {
                gchar * old_mem = result;
                result = g_strdup_printf("%s%s'%s'",result,(*result) ? "," : "",item->name);
                g_free(old_mem);
            }
        }
    }
    return result;
}

////////////////////////////////////
////////////////////////////////////
////////////////////////////////////

static int delete_callback(void * result, int argc, char ** argv, char ** azColName)
{
    delete_callback_data * data = result;
    if(argc >= 4 && result && data->max_delete > data->deleted)
    {
        /* God, this is so silly.. SQL, why you don't like " = null"
         * I can't think of any easier way to do this, tell me if you found one
         */
        gchar * sql_delete = sqlite3_mprintf(sqlcode[SQL_ACTUAL_DELETE],
                argv[0] ? "=" : " IS ", argv[0] ? argv[0] : "NULL",
                argv[1] ? "=" : " IS ", argv[1] ? argv[1] : "NULL",
                argv[2] ? "=" : " IS ", argv[2] ? argv[2] : "NULL",
                argv[3] ? "=" : " IS ", argv[3] ? argv[3] : "NULL",
                argv[4] ? "=" : " IS ", argv[4] ? argv[4] : "NULL");

        if(sql_delete != NULL)
        {
            execute(data->con,sql_delete);
            sqlite3_free(sql_delete);
            data->deleted++;
        }
    }
    return 0;
}
