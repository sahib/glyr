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
#include <glib.h>

/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/

/* Check if a cache is already in the db, by cheskum or source_url  */
bool db_contains(GlyrDatabase * db, GlyrMemCache * cache)
{
	gboolean result = FALSE;
	if(db && cache)
	{
		gchar * sql = sqlite3_mprintf(
				"SELECT source_url,data_checksum,data_size,data_type FROM metadata AS m  "
				"WHERE (m.data_type = %d AND m.data_size = %d AND m.data_checksum = '?') "
				"OR (m.source_url LIKE '%q' AND m.source_url is not null AND data_type = %d);",
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
				fprintf(stderr,"glyr_db_contains: error message: %s\n", sqlite3_errmsg(db->db_handle));
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


