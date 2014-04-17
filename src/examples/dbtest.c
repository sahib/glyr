#include <glyr/glyr.h>
#include <glyr/cache.h>
#include <glib.h>
#include <stdlib.h>

void do_insert(GlyrQuery *q, GlyrDatabase *db, char *artist, char *album)
{
    glyr_query_init(q);
    glyr_opt_artist(q, artist);
    glyr_opt_album(q, album);
    glyr_opt_type(q, GLYR_GET_COVERART);

    GlyrMemCache *c = glyr_cache_new();
    glyr_cache_set_data(c, g_strdup("Dummy"), -1);
    c->rating = -1;
    //    c->dsrc = g_strdup("Hello World"); // Argh.

    glyr_db_insert(db, q, c);
    glyr_cache_free(c);
}

void do_lookup(GlyrQuery *q, GlyrDatabase *db)
{
    GlyrMemCache *c = glyr_db_lookup(db, q);
    g_assert(c);
    glyr_cache_print(c);
    g_assert(c->rating == -1);
    glyr_cache_free(c);
}


int main(int argc, char const *argv[])
{
    system("rm /home/chris/metadata.db");
    glyr_init();
    atexit(glyr_cleanup);

    GlyrDatabase *db = glyr_db_init("/home/chris");
    g_assert(db);

    GlyrQuery q1, q2;

    do_insert(&q1, db, "Equilibrium", "Sagas");
    do_insert(&q2, db, "Letzte Instanz", "Kalter Glanz");

    do_lookup(&q1, db);
    do_lookup(&q2, db);

    glyr_query_destroy(&q1);
    glyr_query_destroy(&q2);
    glyr_db_destroy(db);
    return EXIT_SUCCESS;
}
