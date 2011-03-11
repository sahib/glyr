/***********************************************************
* This is a brief example on how to use libglyr's interface
* This is not a complete documentation, rather a quickstart
* 
* Compile with: gcc example.c -o example -lglyr 
************************************************************/

#include <stdlib.h>
#include <stdio.h>

// replace this with <glyr.h> if you installed it
// This is only to prevent you from installing it
// If you have it locale you may need to add -L../bin to your gcc line
#include "../lib/glyr.h"

static int funny_callback(GlyMemCache * c, GlyQuery * q)
{
	// This is called whenever glyr gets a ready to use item
	// This may be more than one -- see the number option!
	fprintf(stderr,"Received %d bytes of awesomeness.\n",c->size);

	//also:GLYRE_STOP_BY_CB;
	return GLYRE_OK;
}

int main(int argc, char * argv[])
{
	// Initialize a new query
	GlyQuery q;
	Gly_init_query(&q);

	// Set the type 
	GlyOpt_type(&q,GET_LYRIC);

	// Set at least the required fields to your needs
	// For lyrics those are 'artist' and 'title' ('album') 
	// is strictly optional and may be used by a few plugins
	GlyOpt_artist(&q,"Die Apokalyptischen Reiter");
	GlyOpt_album (&q,"Moral & Wahnsinn");
	GlyOpt_title (&q,"Die Boten");

	GlyOpt_dlcallback(&q,funny_callback,NULL);

	// For the start: Enable verbosity
	GlyOpt_verbosity(&q,0);

	// Call the most important command: GET!
	// This returned a list of (GlyMemCache *)s
	// Each containing ONE item. (i.e. a songtext)
	// The list is terminmated by a NULL pointer.
	GlyCacheList * result_list = Gly_get(&q,NULL);
	
	// Now iterate through it...
	if(result_list != NULL)
	{
		fprintf(stderr,"Got %d item(s).\n",result_list->size);

		int i = 0;
		for(i = 0; i < result_list->size; i++)
		{
			fprintf(stderr,"Item from <%s>\nof size %d bytes containing a ",result_list->list[i]->dsrc,result_list->list[i]->size);
			switch(result_list->list[i]->type)
			{
				case TYPE_COVER:     fprintf(stderr,"cover.");
				     break;
				case TYPE_LYRICS:    fprintf(stderr,"songtext.");
				     break;
				case TYPE_PHOTOS:    fprintf(stderr,"band photo.");
				     break;
				case TYPE_REVIEW:    fprintf(stderr,"album Review");
				     break;
				case TYPE_AINFO:     fprintf(stderr,"artistbio");
				     break;
				case TYPE_SIMILIAR:  fprintf(stderr,"similiar artist");
				     break;
				case TYPE_TRACK:     fprintf(stderr,"a trackname.");
				     break;
				case TYPE_NOIDEA:    
				default: fprintf(stderr,"brunette giraffe..?");
			}
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
