#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../../lib/glyr.h"

void * killer_thread(void * arg)
{
    GlyrQuery * q = arg;
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

    glyr_opt_type(&q,GLYR_GET_COVERART);
    glyr_opt_artist(&q,"Equilibrium");
    glyr_opt_album(&q,"Sagas");
    glyr_opt_number(&q,42);
    glyr_opt_verbosity(&q,3);

    g_printerr("-- Starting GET\n");

    GThread * killer = g_thread_create(killer_thread,(gpointer)&q,TRUE,NULL);

    gint len = 0;
    GlyrMemCache * list = glyr_get(&q,NULL,&len);    
    if(list != NULL)
    {
        glyr_free_list(list);
    }
    glyr_query_destroy(&q);
    g_printerr("Got %d size\n",len);
    g_printerr("-- Ending GET\n");
    g_thread_join(killer);

    return EXIT_SUCCESS;
}
