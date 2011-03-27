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

// The one and only..
#include "../lib/glyr.h"

// Just print an item..
static void print_item(GlyMemCache * cacheditem, int num)
{
    // GlyMemcache members
    // dsrc = Exact link to the location where the data came from
    // size = size in bytes
    // type = Type of data
    // data = actual data
    // (error) - Don't use this. Only internal use
    fprintf(stderr,"----- ITEM #%d ------\n",num);
    fprintf(stderr,"FROM: <%s>\n",cacheditem->dsrc);
    fprintf(stderr,"SIZE: %d Bytes\n",(int)cacheditem->size);
    fprintf(stderr,"TYPE: ");

    // Each cache identfies it's data by a constant
    switch(cacheditem->type)
    {
    case TYPE_COVER:
        fprintf(stderr,"cover");
        break;
    case TYPE_COVER_PRI:
        fprintf(stderr,"cover (frontside)");
        break;
    case TYPE_COVER_SEC:
        fprintf(stderr,"cover (backside or inlet)");
        break;
    case TYPE_LYRICS:
        fprintf(stderr,"songtext");
        break;
    case TYPE_PHOTOS:
        fprintf(stderr,"band photo");
        break;
    case TYPE_REVIEW:
        fprintf(stderr,"albumreview");
        break;
    case TYPE_AINFO:
        fprintf(stderr,"artistbio");
        break;
    case TYPE_SIMILIAR:
        fprintf(stderr,"similiar artist");
        break;
    case TYPE_TRACK:
        fprintf(stderr,"trackname [%d:%02d]",cacheditem->duration/60,cacheditem->duration%60);
        break;
    case TYPE_ALBUMLIST:
        fprintf(stderr,"albumname");
        break;
    case TYPE_TAGS:
        fprintf(stderr,"some tag");
        break;
    case TYPE_TAG_ARTIST:
        fprintf(stderr,"artisttag");
        break;
    case TYPE_TAG_ALBUM:
        fprintf(stderr,"albumtag");
        break;
    case TYPE_TAG_TITLE:
        fprintf(stderr,"titletag");
        break;
    case TYPE_RELATION:
        fprintf(stderr,"relation");
        break;
    case TYPE_NOIDEA:
    default:
        fprintf(stderr,"brunette giraffe..? No idea.");
    }

    // Print the actual data.
    // This might have funny results if using cover/photos
    if(!cacheditem->is_image)
        fprintf(stderr,"\nDATA:\n%s",cacheditem->data);
    else
        fprintf(stderr,"\nDATA: <not printable>");

    fprintf(stderr,"\n");
}

int callback_itemctr = 0;

static int funny_callback(GlyMemCache * c, GlyQuery * q)
{
    // This is called whenever glyr gets a ready to use item
    print_item(c,callback_itemctr++);
    return GLYRE_OK;
}

int main(int argc, char * argv[])
{
    // Initialize a new query (this may allocate memory)
    GlyQuery q;
    Gly_init_query(&q);

    // Default to lyrics..
    int type = GET_LYRIC;
    if(argc > 1)
    {
        GlyPlugin * list_of_getters = Gly_get_provider_by_id(GET_UNSURE);
        if(list_of_getters != NULL)
        {
            int i;

            // The end of the commandlist is marked by a NULL in the name
            for(i = 0; list_of_getters[i].name; i++)
            {
                // If it matches the argument...
                if(!strcmp(list_of_getters[i].name,argv[1]))
                {
                    //.. we set the type. The type is stored in the 'gid' variable
                    // (which stores informations about groups elsewhere)
                    type = list_of_getters[i].gid;
                    break;
                }
            }
            // The table is a dyn. allocated copy of the original -> free!
            free(list_of_getters);
        }
    }

    // Now set the type we determined.
    GlyOpt_type(&q,type);

    // Set at least the required fields to your needs
    // For lyrics those are 'artist' and 'title', ('album')
    // is strictly optional and may be used by a few plugins
    GlyOpt_artist(&q,(char*)"Die Apokalyptischen Reiter");
    GlyOpt_album (&q,(char*)"Riders on the Storm");
    GlyOpt_title (&q,(char*)"Friede sei mit dir");

    // Execute a func when getting one item
    GlyOpt_dlcallback(&q,funny_callback,NULL);

    // For the start: Enable verbosity
    GlyOpt_verbosity(&q,2);

    // Download 5 items
    GlyOpt_number(&q,111);

    // Just search
    GlyOpt_download(&q,1);

    // Call the most important command: GET!
    // This returned a list of (GlyMemCache *)s
    // Each containing ONE item. (i.e. a songtext)
    enum GLYR_ERROR err;
    GlyCacheList * result_list = Gly_get(&q,&err);

    if(err != GLYRE_OK)
        fprintf(stderr,"E:%s\n",Gly_strerror(err));

    // Now iterate through it...
    if(result_list != NULL)
    {
        fprintf(stderr,"\n--------------------\n");
        fprintf(stderr,"In total %d item(s).\n",(int)result_list->size);

        size_t i = 0;
        for(i = 0; i < result_list->size; i++)
        {
            // This has the same effect as in the callback,
            // Just that it's executed just once after all DL is done.
            //print_item(Gly_clist_at(result_list,i),(int)i);
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
