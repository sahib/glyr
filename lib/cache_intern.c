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
#include "core.h"
#include "glyr.h"
#include "cache.h"
#include "cache_intern.h"
#include <glib.h>

/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/

/* Check if a cache is already in the db, by cheskum or source_url  */
gboolean db_contains(GlyrDatabase * db, GlyrMemCache * cache)
{
	gboolean result = FALSE;
	if(db && cache)
	{
		gchar * sql = sqlite3_mprintf(
				"SELECT source_url,data_checksum,data_size,data_type FROM metadata AS m      "
				"WHERE (m.data_type = %d AND m.data_size = %d AND m.data_checksum = '?')     "
				"OR (m.source_url LIKE '%q' AND m.source_url IS NOT NULL AND data_type = %d) "
                "LIMIT 1;                                                                    ",
				cache->type,
				cache->size,
				cache->dsrc,
				cache->type);

		if(sql != NULL)
		{
			sqlite3_stmt * stmt = NULL;
			sqlite3_prepare_v2(db->db_handle, sql, strlen(sql) + 1, &stmt, NULL);
			sqlite3_bind_blob(stmt, 1, cache->md5sum, sizeof cache->md5sum, SQLITE_TRANSIENT);
	
			int err = sqlite3_step(stmt);	
			if(err == SQLITE_ROW)
			{
				result = TRUE;
			}
			else if(err != SQLITE_DONE) 
			{
				fprintf(stderr,"db_contains: error message: %s\n", sqlite3_errmsg(db->db_handle));
			}
	
			sqlite3_finalize(stmt);
			sqlite3_free(sql);
		}
	}
	return result;
}

/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/

#define UNLOCK_NOTIFY_MINVERSION 3007000

#if SQLITE_VERSION_NUMBER >= UNLOCK_NOTIFY_MINVERSION

/* This has been shameless ripped of from 
 * http://www.sqlite.org/unlock_notify.html
 * and converted to GLib's threading system.
 */

/*
 ** A pointer to an instance of this structure is passed as the user-context
 ** pointer when registering for an unlock-notify callback.
 */
typedef struct _UnlockNotification 
{
    gboolean fired;           /* True after unlock event has occurred */
    GCond  * cond;            /* Condition variable to wait on */
    GMutex * mutex;           /* Mutex to protect structure */
} UnlockNotification;

/*-----------------------------------------------------------------------*/

/*
 ** This function is an unlock-notify callback registered with SQLite.
 */
static void unlock_notify_cb(void ** apArg, int nArg)
{
    for(gint i = 0; i < nArg; i++)
    {
        UnlockNotification * p = (UnlockNotification *)apArg[i];
        g_mutex_lock(p->mutex);
        p->fired = TRUE;
        g_cond_signal(p->cond);
        g_mutex_unlock(p->mutex);
    }
}


/*-----------------------------------------------------------------------*/

/*
 ** This function assumes that an SQLite API call (either sqlite3_prepare_v2() 
 ** or sqlite3_step()) has just returned SQLITE_LOCKED. The argument is the
 ** associated database connection.
 **
 ** This function calls sqlite3_unlock_notify() to register for an 
 ** unlock-notify callback, then blocks until that callback is delivered 
 ** and returns SQLITE_OK. The caller should then retry the failed operation.
 **
 ** Or, if sqlite3_unlock_notify() indicates that to block would deadlock 
 ** the system, then this function returns SQLITE_LOCKED immediately. In 
 ** this case the caller should not retry the operation and should roll 
 ** back the current transaction (if any).
 */
static gint wait_for_unlock_notify(sqlite3 *db)
{
    gint rc;
    UnlockNotification un;

    un.mutex = NULL;
    un.cond  = NULL;

    /* Initialize the UnlockNotification structure. */
    un.fired = FALSE;
    un.mutex = g_mutex_new();
    un.cond  = g_cond_new();

    /* Register for an unlock-notify callback. */
    rc = sqlite3_unlock_notify(db, unlock_notify_cb,(void *)&un);
    g_assert(rc == SQLITE_LOCKED || rc == SQLITE_OK);

    /* The call to sqlite3_unlock_notify() always returns either SQLITE_LOCKED 
     ** or SQLITE_OK. 
     **
     ** If SQLITE_LOCKED was returned, then the system is deadlocked. In this
     ** case this function needs to return SQLITE_LOCKED to the caller so 
     ** that the current transaction can be rolled back. Otherwise, block
     ** until the unlock-notify callback is invoked, then return SQLITE_OK.
     */
    if(rc == SQLITE_OK)
    {
        g_mutex_lock(un.mutex);
        if(un.fired == FALSE)
        {
            g_cond_wait(un.cond, un.mutex);
        }
        g_mutex_unlock(un.mutex);
    }

    /* Destroy the mutex and condition variables. */
    g_cond_free(un.cond);
    g_mutex_free(un.mutex);
    return rc;
}

#endif

/*-----------------------------------------------------------------------*/

/*
 ** This function is a wrapper around the SQLite function sqlite3_step().
 ** It functions in the same way as step(), except that if a required
 ** shared-cache lock cannot be obtained, this function may block waiting for
 ** the lock to become available. In this scenario the normal API step()
 ** function always returns SQLITE_LOCKED.
 **
 ** If this function returns SQLITE_LOCKED, the caller should rollback
 ** the current transaction (if any) and try again later. Otherwise, the
 ** system may become deadlocked.
 */
gint sqlite3_blocking_step(sqlite3_stmt *pStmt)
{
    gint rc;

#if SQLITE_VERSION_NUMBER >= UNLOCK_NOTIFY_MINVERSION 

    while(SQLITE_LOCKED == (rc = sqlite3_step(pStmt)))
    {
        rc = wait_for_unlock_notify(sqlite3_db_handle(pStmt));
        if(rc != SQLITE_OK)
        {
            break;
        }
        sqlite3_reset(pStmt);
    }

#else 

    rc = sqlite3_step(pStmt);    

#endif

    return rc;
}

/*-----------------------------------------------------------------------*/

gint sqlite3_blocking_prepare_v2(sqlite3 *db, const gchar *zSql, gint nSql, sqlite3_stmt **ppStmt, const char **pz)
{
    gint rc;

#if SQLITE_VERSION_NUMBER >= UNLOCK_NOTIFY_MINVERSION 

    while(SQLITE_LOCKED == (rc = sqlite3_prepare_v2(db, zSql, nSql, ppStmt, pz)))
    {
        rc = wait_for_unlock_notify(db);
        if(rc != SQLITE_OK)
        {
            break;
        }
    }

#else

    rc = sqlite3_prepare_v2(db,zSql,nSql,ppStmt,pz);

#endif

    return rc;
}

/*-----------------------------------------------------------------------*/


gint sqlite3_blocking_exec(sqlite3 * db, const gchar * sql_code, 
                     int (*callback)(void*,int,char**,char**), 
                     void * user_ptr, gchar ** err_msg)
{
    gint rc;

#if SQLITE_VERSION_NUMBER >= UNLOCK_NOTIFY_MINVERSION 

    while(SQLITE_LOCKED == (rc = sqlite3_exec(db, sql_code, callback, user_ptr, err_msg)))
    {
        rc = wait_for_unlock_notify(db);
        if(rc != SQLITE_OK)
        {
            break;
        }
    }

#else

    rc = sqlite3_exec(db,sql_code,callback,user_ptr,err_msg);
    
#endif

    return rc;
}
