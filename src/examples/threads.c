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

#include <stdlib.h>
#include <stdio.h>

#include <glib.h>
#include "../../lib/glyr.h"
#include "../../lib/cache.h"

/*
 * Example program showing how to use libglyr with multiple threads.
 * libglyr is fully thread-compatible as long every thread uses it's own query struct.
*/

static void * call_get(void * p)
{
    GlyrMemCache * r = glyr_get(p,NULL,NULL);
    puts("Thread finished");
    return r;
}

static void configure(GlyrQuery * s, GLYR_GET_TYPE type)
{
    glyr_query_init(s);
    glyr_opt_artist(s,"Die Apokalyptischen Reiter");
    glyr_opt_album (s,"Moral & Wahnsinn");
    glyr_opt_title (s,"Die Boten");
    glyr_opt_verbosity(s,1);
    glyr_opt_type  (s,type);
    glyr_opt_from(s,NULL);
}

static void printme(GlyrMemCache * start)
{
    GlyrMemCache * i = start;
    while(i != NULL)
    {
        glyr_cache_write(i,"stdout");
        i = i->next;
    }
    glyr_free_list(start);
}

int main(void)
{
    /* Init */
    glyr_init();
    atexit(glyr_cleanup);

    /* Fill some silly values */
    GlyrQuery query_long, query_longer;
    configure(&query_long,GLYR_GET_SIMILIAR_ARTISTS);
    configure(&query_longer,GLYR_GET_LYRICS);

    GlyrDatabase * db = glyr_db_init("/tmp");
    glyr_opt_lookup_db(&query_long,db);
    glyr_opt_lookup_db(&query_longer,db);

    GlyrMemCache * a, * b;

    /* Create two threads */
    GThread * thread_a = g_thread_create(call_get,(gpointer)&query_long,true,NULL);
    GThread * thread_b = g_thread_create(call_get,(gpointer)&query_longer,true,NULL);

    puts("Running!");

    /* Wait for threads to join, get results */
    a = g_thread_join(thread_a);
    b = g_thread_join(thread_b);

    puts("Joined!\n---> Data: ");

    if(a && b)
    {
        printme(a);
        puts("-------------");
        printme(b);
    }

    glyr_query_destroy(&query_long);
    glyr_query_destroy(&query_longer);
    return EXIT_SUCCESS;
}
