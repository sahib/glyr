#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cover.h"
#include "lyrics.h"

int main(int argc, char **argv)
{
    // ToDo: cmd line parsing
    if(argc > 1)
    {
	if(!strcmp(argv[1],"cover"))
	{
	    if (argc > 5)
	    {
		char *path = get_cover(argv[2],argv[3],".",0,atoi(argv[3]),argv[5]);

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
		char *path = get_lyrics(argv[2],argv[3],argv[4],".",1,10,NULL);

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
    return EXIT_FAILURE;
}
