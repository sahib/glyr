#include <time.h>
#include <stdio.h>

#include "../../lib/glyr.h"
#include "../../lib/cache.h"

/* Short example to show a more obscure feature of caching:
 * Every cache has a field called 'rating' - see below:
 *
 * Execute this example twice (./bin/rating [how_many_items]:
 * 1) First time some URLs will be found in the web. 
 *    A random rating is applied to each, and they're written to the DB.
 * 2) The second run will get the URLs from the cache instead. Sorted by rating,
 *    and if two have the same rating the last added will be the first (sorted by timestamp).
 *    Newly found (== not yet cached items) are always at the very end of the list.
 * 3) will yield the same result as 2)
 * 4) 'rm /tmp/metadata.db' to start again.
 */

int main(int argc, const char *argv[])
{
    int amount_to_get = (argc > 1) ? strtol(argv[1],NULL,10) : 3;
    GLYR_ERROR err;

    glyr_init();
    atexit(glyr_cleanup);

    srand(time(NULL));

    GlyrDatabase * db = glyr_db_init("/tmp");
    if(db != NULL)
    {
        GlyrQuery q;
        glyr_query_init(&q);
        glyr_opt_artist(&q,"Die Apokalyptischen Reiter");
        glyr_opt_type(&q,GLYR_GET_ARTIST_PHOTOS);
        glyr_opt_download(&q,false);
        glyr_opt_number(&q,amount_to_get);

        glyr_opt_lookup_db(&q,db);
    
        /* Say, we want to manage the writing part ourself */
        glyr_opt_db_autowrite(&q,false);

        /* Now either get me some from the web or the db */
        GlyrMemCache * list = glyr_get(&q,&err,NULL);
        if(err != GLYRE_OK)
        {
            fprintf(stderr,"Error occured: %s\n",glyr_strerror(err));
        }

        if(list != NULL)
        {
            for(GlyrMemCache * cache = list; cache; cache = cache->next)
            {
                puts("-----------------");
                glyr_cache_print(cache);
                puts("-----------------");

                /* Give it some rating if not already cached */
                if(cache->cached == false)
                {
                    cache->rating = rand() % 100;
                    glyr_db_insert(db,&q,cache);
                }
            }
            glyr_free_list(list);
        }
        glyr_query_destroy(&q);
        glyr_db_destroy(db);
    }
    return EXIT_SUCCESS;
}
