/***********************************************************
 * This file is part of glyr
 * + a commnandline tool and library to download various sort of musicrelated metadata.
 * + Copyright (C) [2011-2012]  [Christopher Pahl]
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

#include "../../lib/glyr.h"
#include <glib.h>

/* GLib does not allow it to be NULL */
#define THREAD_TERMINATOR (gpointer)0x1

/*
 * Compile with:
 * gcc threadexample.c -o threadexample $(pkg-config --libs --cflags glib-2.0 gthread-2.0) -lglyr -std=c99 -Wall
 */

/////////////////////////////////

typedef struct _FinishedNotify
{
    gint counter;
    GAsyncQueue * queue;

} FinishedNotify;

/////////////////////////////////

/* Our polling thread */
gpointer apollo_orbiter (gpointer gp_queue)
{
    FinishedNotify * pn = gp_queue;
    g_async_queue_ref (pn->queue);

    while (TRUE)
    {
        g_print ("\n-----------\n\n");
        gpointer thread_data = g_async_queue_pop (pn->queue);

        if (thread_data == THREAD_TERMINATOR)
        {
            g_printerr ("\n-- Terminating --\n");
            break;
        }
        else
        {
            GlyrQuery * q = thread_data;
            GlyrMemCache * head = glyr_get (q,NULL,NULL);
            if (head != NULL)
            {
                g_print ("//////// ITEM %d ////////\n",++pn->counter);
                glyr_cache_print (head);
                glyr_free_list (head);
                g_print ("/////////////////////////\n");
            }
            glyr_query_destroy (q);
        }
    }

    g_async_queue_unref (pn->queue);
    return NULL;
}

/////////////////////////////////

void build_queries (GlyrQuery * one, GlyrQuery * two, GlyrQuery * three)
{
    glyr_query_init (one);
    glyr_query_init (two);
    glyr_query_init (three);

    glyr_opt_artist (one,"Diablo Swing Orchestra");
    glyr_opt_title (one,"Balrog Boogie");
    glyr_opt_type  (one, GLYR_GET_LYRICS);

    glyr_opt_artist (two, "Farin Urlaub");
    glyr_opt_download (two,FALSE);
    glyr_opt_type  (two, GLYR_GET_ARTIST_PHOTOS);

    glyr_opt_artist (three, "Knorkator");
    glyr_opt_type  (three, GLYR_GET_ARTIST_PHOTOS);
}

/////////////////////////////////

gint sort_async_queue_jobs (gconstpointer a, gconstpointer b, gpointer data)
{
    return (a == THREAD_TERMINATOR) ? -1 : (b == THREAD_TERMINATOR) ? 1 : 0;
}

/////////////////////////////////

int main (void)
{
    glyr_init();
    atexit (glyr_cleanup);

    /* Create a new async queue */
    FinishedNotify notify;
    notify.queue = g_async_queue_new();
    notify.counter = 0;

    /* Initialize a new thread */
    GError * err = NULL;
    GThread * apollo = g_thread_new("apollo", apollo_orbiter, &notify);

    if (apollo != NULL)
    {
        /* Push a few jobs */
        GlyrQuery one,two,three;
        build_queries (&one,&two,&three);
        g_async_queue_push (notify.queue,&one  );
        g_async_queue_push (notify.queue,&two  );
        g_async_queue_push (notify.queue,&three);

#define ABORT_IMMEDIATELY FALSE

#if ABORT_IMMEDIATELY
        /* Test if it really aborts immediately, if not it crashes :-) */
        g_async_queue_push (notify.queue, (gpointer) 0x2);
#endif

        /* Terminate by pushing a special value */
        g_async_queue_push (notify.queue,THREAD_TERMINATOR);

#if ABORT_IMMEDIATELY
        /* Sort the THREAD_TERMINATOR before anything else. */
        g_async_queue_sort (notify.queue,sort_async_queue_jobs,NULL);
#endif

        /* Wait till he finished */
        g_thread_join (apollo);
    }
    else
    {
        g_printerr ("Could not create thread: %s\n",err->message);
        g_error_free (err);
    }

    g_async_queue_unref (notify.queue);
    return EXIT_SUCCESS;
}
