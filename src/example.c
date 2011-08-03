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
#include "../lib/glyr.h"


static void print_item(GlyQuery *s, GlyMemCache * cacheditem, int num)
{
	fprintf(stderr,"\n------- ITEM #%d --------\n",num);
	Gly_printitem(s,cacheditem);
	fprintf(stderr,"\n------------------------\n");
}

/* ------------------------------------------ */

/* This is called whenever glyr gets a ready to use item */
static enum GLYR_ERROR funny_callback(GlyMemCache * c, GlyQuery * q)
{
    /* You can pass a void pointer to the callback,
     * by passing it as third argument to GlyOpt_dlcallback()
     */
    int * i = q->callback.user_pointer;
    *i = *i + 1;

    if(*i == 3)
    {
        puts("=> Gentlemen, we received 3 items. We should stop now.");
        return GLYRE_STOP_BY_CB;
	/*
         * You can also return:
         * - GLYRE_STOP_BY_CB which will stop libglyr
         * - GLYRE_IGNORE which will cause libglyr not to add this item to the results
         */
    }
    return GLYRE_OK;
}

/* ------------------------------------------ */

int main(int argc, char * argv[])
{
    // Initialize a new query (this may allocate memory)
    GlyQuery q;
    Gly_init_query(&q);

    /* You need to call this before anything happens */
    //Gly_init();
    //atexit(Gly_cleanup);

    /* Say we want a Songtext */
    enum GLYR_GET_TYPE type = GET_LYRICS;
    GlyOpt_type(&q,type);

    // Set at least the required fields to your needs
    // For lyrics those are 'artist' and 'title', ('album')
    // is strictly optional and may be used by a few plugins
    GlyOpt_artist(&q,(char*)"Die Apokalyptischen Reiter");
    GlyOpt_album (&q,(char*)"Riders on the Storm");
    GlyOpt_title (&q,(char*)"Friede sei mit dir");

    // Execute a func when getting one item
    int this_be_my_counter = 0;
    GlyOpt_dlcallback(&q,funny_callback,&this_be_my_counter);

    // For the start: Enable verbosity
    GlyOpt_verbosity(&q,2);

    // Download 5 items
    GlyOpt_number(&q,5);

    // Just search, without downloading items
    GlyOpt_download(&q,1);

    // Call the most important command: GET!
    // This returned a list of (GlyMemCache *)s
    // Each containing ONE item. (i.e. a songtext)
    enum GLYR_ERROR err;
    GlyMemCache * it = Gly_get(&q,&err,NULL);

    if(err != GLYRE_OK)
        fprintf(stderr,"E:%s\n",Gly_strerror(err));

    // Now iterate through it...
    if(it != NULL)
    {
        GlyMemCache * start = it;

        int counter = 0;
        while(it != NULL)
        {
            // This has the same effect as in the callback,
            // Just that it's executed just once after all DL is done.
            // Commented out, as this would print it twice
	    print_item(&q,it,counter);

            // Every cache has a link to the next and prev one (or NULL respectively)
            it = it->next;
            ++counter;
        }

        // The contents of it are dynamically allocated.
        // So better free them if you're not keen on memoryleaks
        Gly_free_list(start);
    }
    // Destroy query (reset to default values and free dyn memory)
    // You could start right off to use this query in another Gly_get
    Gly_destroy_query(&q);
    return EXIT_SUCCESS;
}
