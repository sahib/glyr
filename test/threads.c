#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>
#include "<glyr.h>"

/* 
 * Example program showing how to use libglyr with multiple threads.
 * Compile: gcc threads.c -o threads -pthread -L../bin -lglyr -Wall -Wextra
 * libglyr is fully thread-compatible as long every thread uses it's own query struct.
 * (Honestly: Something else won't make much sense....)
*/

static void * call_get(void * p)
{
    return Gly_get(p,NULL);
}

static void configure(GlyQuery * s, enum GLYR_GET_TYPE type)
{
    Gly_init_query(s);
    GlyOpt_artist(s,"Die Apokalyptischen Reiter");
    GlyOpt_album (s,"Moral & Wahnsinn");
    GlyOpt_title (s,"Die Boten");
    GlyOpt_verbosity(s,2);
    GlyOpt_type  (s,type);
}

int main(void)
{
        /* Init */
        Gly_init();
        atexit(Gly_cleanup);

        pthread_t agent_long,agent_longer;
        GlyQuery query_long, query_longer;
       
        /* Fill some silly values */ 
        configure(&query_long,GET_SIMILIAR);
        configure(&query_longer,GET_LYRIC);

        GlyCacheList * a, * b;
        
        /* Create two threads */
        pthread_create(&agent_long,  NULL,call_get,&query_long  );
        pthread_create(&agent_longer,NULL,call_get,&query_longer);

        /* Wait for threads to join, get results */
        pthread_join(agent_long,  (void**)&a);
        pthread_join(agent_longer,(void**)&b);

        if(a && b)
        {
                /* Now print the results. */
                size_t i = 0;
                for(i = 0; i < a->size; i++)
                    Gly_write(&query_long,Gly_clist_at(a,i),"stdout");
                puts("-------------");
                for(i = 0; i < b->size; i++)
                    Gly_write(&query_longer,Gly_clist_at(b,i),"stdout");

                Gly_free_list(a);
                Gly_free_list(b);
        }

        Gly_destroy_query(&query_long);
        Gly_destroy_query(&query_longer);
        return EXIT_SUCCESS;
}
