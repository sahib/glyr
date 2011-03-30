#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>
#include "../lib/glyr.h"

/* 
 * Example program showing how to use libglyr with multiple threads.
 * Compile: gcc threads.c -o threads -pthread -L../bin -lglyr -Wall -Wextra
 * libglyr is fully thread-compatible as long every thread uses it's own query struct.
 * (Honestly: Something else won't make much sense....)
*/

void * call_get(void * p)
{
    return Gly_get(p,NULL);
}

int main(void)
{
        Gly_init();
        atexit(Gly_cleanup);

        pthread_t agent_long;
        pthread_t agent_longer;
        
        GlyQuery query_long;
        Gly_init_query(&query_long);
        
        GlyQuery query_longer;
        Gly_init_query(&query_longer);

        GlyOpt_artist(&query_long,"Equilibrium");
        GlyOpt_artist(&query_longer,"Equilibrium");
        GlyOpt_title(&query_longer,"Wurzelbert");

        GlyOpt_type(&query_long,GET_SIMILIAR);
        GlyOpt_type(&query_longer,GET_LYRIC);

        GlyOpt_verbosity(&query_long,2);
        GlyOpt_verbosity(&query_longer,2);

        GlyCacheList * a, * b;
        pthread_create(&agent_long,  NULL,call_get,&query_long  );
        pthread_create(&agent_longer,NULL,call_get,&query_longer);

        pthread_join(agent_long,(void**)&a);
        pthread_join(agent_longer,(void**)&b);

        if(a && b)
        {
                size_t i = 0;
                for(i = 0; i < a->size; i++)
                    Gly_write(&query_long,Gly_clist_at(a,i),"stdout");
                puts("-------------");
                for(i = 0; i < b->size; i++)
                    Gly_write(&query_longer,Gly_clist_at(b,i),"stdout");

                Gly_free_list(a);
                Gly_free_list(b);
        }
        return EXIT_SUCCESS;
}
