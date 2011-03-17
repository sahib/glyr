/***********************************************************
* This is a brief example on how to use libglyr's interface
* This is not a complete documentation, rather a quickstart in 100 LOC
*
* Compile with: gcc example.c -o example -lglyr
************************************************************/

#include <stdlib.h>
#include <stdio.h>

// The one and only..
#include "../lib/glyr.h"

static int funny_callback(GlyMemCache * c, GlyQuery * q)
{
    // This is called whenever glyr gets a ready to use item
    fprintf(stderr,"Received %u bytes of awesomeness.\n",(unsigned int)c->size);
    return GLYRE_OK;
}

int main(void)
{
    // Initialize a new query (this may allocate memory)
    GlyQuery q;
    Gly_init_query(&q);

    // Set the type
    // This sample program should work well with other types already ;-)
    // Just zty GET_COVER for example
    GlyOpt_type(&q,GET_LYRIC);

    // Set at least the required fields to your needs
    // For lyrics those are 'artist' and 'title' ('album')
    // is strictly optional and may be used by a few plugins
    GlyOpt_artist(&q,(char*)"Die Apokalyptischen Reiter");
    GlyOpt_album (&q,(char*)"Moral & Wahnsinn");
    GlyOpt_title (&q,(char*)"Die Boten");
    // Execute a func when getting one item
    GlyOpt_dlcallback(&q,funny_callback,NULL);

    // For the start: Enable verbosity
    GlyOpt_verbosity(&q,2);

    // Call the most important command: GET!
    // This returned a list of (GlyMemCache *)s
    // Each containing ONE item. (i.e. a songtext)
    GlyCacheList * result_list = Gly_get(&q,NULL);

    // Now iterate through it...
    if(result_list != NULL)
    {
        fprintf(stderr,"Got %d item(s).\n",(int)result_list->size);

        size_t i = 0;
        for(i = 0; i < result_list->size; i++)
        {
            // GlyMemcache members
            // dsrc = Exact link to the location where the data came from
            // size = size in bytes
            // type = Type of data
            // data = actual data
            // (error) - Don't use this. Only internal use
            fprintf(stderr,"Item from <%s>\nof size %d bytes containing a ",result_list->list[i]->dsrc,(int)result_list->list[i]->size);

            // Each cache identfies it's data by a constant
            switch(result_list->list[i]->type)
            {
            case TYPE_COVER:
                fprintf(stderr,"cover.");
                break;
            case TYPE_LYRICS:
                fprintf(stderr,"songtext.");
                break;
            case TYPE_PHOTOS:
                fprintf(stderr,"band photo.");
                break;
            case TYPE_REVIEW:
                fprintf(stderr,"album Review");
                break;
            case TYPE_AINFO:
                fprintf(stderr,"artistbio");
                break;
            case TYPE_SIMILIAR:
                fprintf(stderr,"similiar artist");
                break;
            case TYPE_TRACK:
                fprintf(stderr,"a trackname.");
                break;
            case TYPE_NOIDEA:
            default:
                fprintf(stderr,"brunette giraffe..?");
            }

            // Print the actual data.
            // This might have funny results if using cover/photos
            fprintf(stderr,"\n\n%s\n",result_list->list[i]->data);
        }
        // The contents of result_list are dynamically allocated.
        // So better free them if you're not keen on memoryleaks
        Gly_free_list(result_list);
        result_list = NULL;
    }
    // Destroy query (reset to default values and free dyn memory)
    // You could start right off to use this query in another Gly_get
    Gly_destroy_query(&q);
    return EXIT_SUCCESS;
}
