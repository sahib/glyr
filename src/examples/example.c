/***********************************************************
 * This file is part of glyr
 * + a command-line tool and library to download various sort of music related metadata.
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

/***********************************************************
* This is a brief example on how to use libglyr's interface
* This is not a complete documentation, rather a quickstart
* It uses nevertheless all important functions and for sure
* everything you will need to hack your own application...
*
* Compile with: gcc example.c -o example -lglyr
************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Usually only this needs to be included */
#include "../../lib/glyr.h"


static void print_item(GlyrMemCache *cacheditem, int num)
{
    fprintf(stderr, "\n------- ITEM #%d --------\n", num);
    glyr_cache_print(cacheditem);
    fprintf(stderr, "\n------------------------\n");
}

/////////////////////////////////

/* This is called whenever glyr gets a ready to use item */
static GLYR_ERROR funny_callback(GlyrMemCache *c, GlyrQuery *q)
{
    /* You can pass a void pointer to the callback,
     * by passing it as third argument to glyr_opt_dlcallback()
     */
    int *i = q->callback.user_pointer;

    if(*i == 1) {
        puts("=> Gentlemen, we received an item.");
        puts("=> We originally wanted more, but we decide to stop here.");
        puts("=> Therefore we return GLYRE_STOP_PRE. Goodbye.");
        return GLYRE_STOP_PRE;
        /*
         * You can also return:
         * - GLYRE_STOP_POST which will stop libglyr, but still add the current item
         * - GLYRE_STOP_PRE  which will stop libglyr, but skip the current item
         * - GLYRE_SKIP which will cause libglyr not to add this item to the results
         */
    }
    *i = *i + 1;
    return GLYRE_OK;
}

/////////////////////////////////

int main(int argc, char *argv[])
{
    /* You need to call this before anything happens */
    glyr_init();
    atexit(glyr_cleanup);

    /* Initialize a new query (this may allocate memory) */
    GlyrQuery q;
    glyr_query_init(&q);

    /* Say we want a Songtext */
    GLYR_GET_TYPE type = GLYR_GET_LYRICS;
    glyr_opt_type(&q, type);

    /* Set at least the required fields to your needs        *
     * For lyrics those are 'artist' and 'title', ('album')  *
     * is strictly optional and may be used by a few plugins */
    glyr_opt_artist(&q, (char *) "Die Apokalyptischen Reiter");
    glyr_opt_album(&q, (char *) "Riders on the Storm");
    glyr_opt_title(&q, (char *) "Friede sei mit dir");

    /* Execute a func when getting one item */
    int this_be_my_counter = 0;
    glyr_opt_dlcallback(&q, funny_callback, &this_be_my_counter);

    /* For the start: Enable verbosity */
    glyr_opt_verbosity(&q, 2);

    /* Download 5 (or less) items */
    glyr_opt_number(&q, 5);

    /* Just search, without downloading items */
    glyr_opt_download(&q, 0);

    /* Call the most important command: GET!
     * This returned a list of (GlyrMemCache *)s
     * Each containing ONE item. (i.e. a songtext)
     */
    GLYR_ERROR err;
    GlyrMemCache *it = glyr_get(&q, &err, NULL);

    if(err != GLYRE_OK) {
        fprintf(stderr, "E:%s\n", glyr_strerror(err));
    }

    /* Now iterate through it... */
    if(it != NULL) {
        GlyrMemCache *start = it;

        int counter = 0;
        while(it != NULL) {
            /* This has the same effect as in the callback,
             * Just that it's executed just once after all DL is done.
             * Commented out, as this would print it twice
             * */
            print_item(it, counter);

            /* Every cache has a link to the next and prev one (or NULL respectively) */
            it = it->next;
            ++counter;
        }

        /* The contents of it are dynamically allocated. */
        /* So better free them if you're not keen on memoryleaks */
        glyr_free_list(start);
    }
    /* Destroy query (reset to default values and free dyn memory) */
    /* You could start right off to use this query in another glyr_get */
    glyr_query_destroy(&q);
    return EXIT_SUCCESS;
}
