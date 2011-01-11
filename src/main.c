#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cover.h"
#include "lyrics.h"

#include "../glyr_config.h"

int main(int argc, char **argv)
{
    // ToDo: cmd line parsing
    if(argc > 1)
    {
	if(!strcmp(argv[1],"cover"))
	{
	    if (argc > 4)
	    {
		char *path = get_cover(argv[2],argv[3],argv[4],0,10,"wla");

		if (path)
		{
		    free(path);
		    return EXIT_SUCCESS;
		}
	    }

	}
	else if(!strcmp(argv[1],"lyric"))
	{
	    if (argc > 5)
	    {
		char *path = get_lyrics(argv[2],argv[3],argv[4],argv[5],1,10,NULL);

		if (path)
		{
		    free(path);
		    return EXIT_SUCCESS;
		}
	    }
	}
	else
	{
		puts("Unknown command");
	}
    }


    fprintf(stderr,"Exit is lacking success...\n");
    fprintf(stderr,"-> %d.%d \n",glyr_VERSION_MAJOR,glyr_VERSION_MINOR); 
    return EXIT_FAILURE;
}
