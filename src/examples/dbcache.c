#include <string.h>
#include <stdio.h>
#include <glib.h>

#include <glyr/glyr.h>
#include <glyr/cache.h>

int main(int argc, char * argv[])
{
	int length = -1;
	GlyrQuery query;
	GlyrMemCache *list = NULL;
	GlyrDatabase *db = NULL;
	GLYR_ERROR get_error = GLYRE_OK;

	glyr_init();

	db = glyr_db_init("/tmp/");

	glyr_query_init(&query);

  glyr_opt_verbosity(&query,2);
	glyr_opt_artist(&query,"Equilibrium");
	glyr_opt_album(&query,"Sagas");
	glyr_opt_title(&query,"Blut im Auge");
	glyr_opt_type(&query,GLYR_GET_LYRICS);
	glyr_opt_lookup_db(&query, db);
	glyr_opt_db_autowrite(&query, 1);

	list = glyr_get(&query, &get_error, &length);

	if (list != NULL)
	{
		for (GlyrMemCache *cache=list;cache!=NULL;cache=cache->next)
		{
			if (cache->cached)
			{
				GlyrMemCache *new = glyr_cache_copy(cache);			
				glyr_cache_set_data(new,"edited\n", -1);
				glyr_cache_print(new);
				glyr_db_edit(db, cache->md5sum, &query, new);			
				glyr_cache_free(new);
			}

			glyr_cache_print(cache);

			if (cache->cached && strcmp(cache->data,"edited\n") == 0)
			{
				glyr_db_delete(db, &query);
			}
		}
	}
	
  glyr_free_list(list);
	glyr_query_destroy(&query);	
	glyr_db_destroy(db);
}
