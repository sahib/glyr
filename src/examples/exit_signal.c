#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../../lib/glyr.h"

/* ----------------------------------------------
 * Example on using the glyr_signal_exit() method.
 * -----------------------------------------------*/

static GLYR_ERROR funny_callback(GlyrMemCache *c, GlyrQuery *q)
{
    glyr_cache_print(c);
    return GLYRE_OK;
}

static void *killer_thread(void *arg)
{
    GlyrQuery *q = arg;
    g_usleep(rand() % 10000000);

    g_printerr("####### SENDING EXIT ######\n");
    glyr_signal_exit(q);
    return NULL;
}

int main(int argc, const char *argv[])
{
    srand(time(NULL));
    glyr_init();
    atexit(glyr_cleanup);

    GlyrQuery q;
    glyr_query_init(&q);

    glyr_opt_type(&q, GLYR_GET_COVERART);
    glyr_opt_artist(&q, "Equilibrium");
    glyr_opt_album(&q, "Sagas");
    glyr_opt_number(&q, 42);
    glyr_opt_verbosity(&q, 3);
    glyr_opt_dlcallback(&q, funny_callback, NULL);

    g_printerr("-- Starting GET\n");

    GThread *killer = g_thread_new("killer_thread", killer_thread, (gpointer) &q);

    gint len = 0;
    GlyrMemCache *list = glyr_get(&q, NULL, &len);
    if(list != NULL) {
        glyr_free_list(list);
    }
    glyr_query_destroy(&q);
    g_printerr("Got %d size\n", len);
    g_printerr("-- Ending GET\n");
    g_thread_join(killer);

    return EXIT_SUCCESS;
}
