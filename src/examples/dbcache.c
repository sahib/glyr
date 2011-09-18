#include <string.h>
#include <stdio.h>
#include <glib.h>

/* Small, cute and confusing example on how to use libglyr's caching.
 * You should execute 3 times: 
 * 1) It will get some lyrics from the web
 * 2) It will get the lyrics from the database, and edit it
 * 3) It will show you the edited cache and delete it from the DB
 * 4) Same as 1) 
 */

#include "../../lib/glyr.h"
#include "../../lib/cache.h"

int main(int argc, char * argv[])
{
	int length = -1;
	GlyrQuery query;
	GlyrMemCache *list = NULL;
	GlyrDatabase *db = NULL;
	GLYR_ERROR get_error = GLYRE_OK;

	/* Initialize..*/
	glyr_init();
	glyr_query_init(&query);

	/* Create a new database in the /tmp folder
         * A new file called 'metadata.db' will pop
         * up there. (GLYR_DB_FILENAME expands to 'metadata.db')
         */
	db = glyr_db_init("/tmp/");

	/* Set some random settings */
	glyr_opt_verbosity(&query,2);
	glyr_opt_artist(&query,"Equilibrium");
	glyr_opt_album(&query,"Sagas");
	glyr_opt_title(&query,"Blut im Auge");
	glyr_opt_type(&query,GLYR_GET_LYRICS);

	/* Tell libglyr to automatically lookup before searching the web */
	glyr_opt_lookup_db(&query, db);

	/* Also tell it to write newly found items to the db */
	glyr_opt_db_autowrite(&query, TRUE);

	list = glyr_get(&query, &get_error, &length);
	if (list != NULL)
	{
		for (GlyrMemCache *cache=list;cache!=NULL;cache=cache->next)
		{
			/* Copy a cache and modify it somehow */
			if (cache->cached)
			{
				GlyrMemCache *new = glyr_cache_copy(cache);
				glyr_cache_set_data(new,g_strdup("edited\n"), -1);

				bool which_edit = false;
				if(which_edit == false)
				{
					/* There are two way to edit it..
                                         * One is to delete all caches according to query, and replace it
                                         * by something else. 
					 */
					glyr_db_edit(db, &query, new);
				}
				else
				{
					/* ..or find a cache via it's md5sum and replace it.
                                         * You may choose one of them.
					 */
					glyr_db_replace(db, cache->md5sum, &query, new);
				}
				glyr_cache_free(new);
			}

			/* Print a nice representation */
			glyr_cache_print(cache);

			/* Now delete it if it was 'edited' */
			if (cache->cached && strcmp(cache->data,"edited\n") == 0)
			{
				glyr_db_delete(db, &query);
			}
		}
	}

	/* Cleanup */
	glyr_free_list(list);
	glyr_query_destroy(&query);	
	glyr_db_destroy(db);
	glyr_cleanup();
}
