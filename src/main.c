#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include "glyr.h"

static void usage(void)
{
    printf("Help text is still missing - Sorry.\n");
    printf("Usage Examples: \n");
    printf("\t./glyr -g cover -o \"Fejd %% Eifur\" --from \"all\" --update\n");
    printf("\t./glyr -g lyric -o \"Fejd %% Eifur\" %% Drängen o kräkan\" --from \"safe\" --update\n");
    printf("\n");
    exit(0);
}

static void parse_commandline(int argc, char * const * argv, glyr_settings_t * glyrs)
{
    int c;
    while (true)
    {
        int option_index = 0;

        static struct option long_options[] =
        {
            {"get",      required_argument, 0, 'g'},
            {"from",     required_argument, 0, 'f'},
            {"of",       required_argument, 0, 'o'},
            {"write",    required_argument, 0, 'w'},
            {"update",   no_argument,       0, 'u'},
            {"help",     no_argument,       0, 'h'},
            {0,          0,                 0,  0 }
        };

        c = getopt_long(argc, argv, "g:f:o:i:a:w:uh",long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
        case 'o':
	    glyr_setopt(glyrs,GLYR_OPT_OF,optarg);
            break;

        case 'w':
	    glyr_setopt(glyrs,GLYR_OPT_PATH,optarg);
            break;

        case 'g':
	    if(!strcmp(optarg,"cover"))
	    	glyr_setopt(glyrs,GLYR_OPT_TYPE,GET_COVER);
	    else if(!strcmp(optarg,"photos"))
	    	glyr_setopt(glyrs,GLYR_OPT_TYPE,GET_PHOTO);
	    else if(!strcmp(optarg,"lyrics"))
	    	glyr_setopt(glyrs,GLYR_OPT_TYPE,GET_LYRIC);
	    else if(!strcmp(optarg,"books"))
	    	glyr_setopt(glyrs,GLYR_OPT_TYPE,GET_BOOKS);
	    else
		printf("Unknown word for --get: '%s'\n",optarg);	
            break;

        case 'u':
	    glyr_setopt(glyrs,GLYR_OPT_UPDATE,true);
            break;

        case 'f':
	    glyr_setopt(glyrs,GLYR_OPT_FROM,optarg);
            break;

        case 'h':
            usage();
            break;
        }
    }

    if (optind < argc)
    {
        printf("Unused strings in cmdline: \n");
        while (optind < argc)
            printf("  '%s'\n", argv[optind++]);

        printf("\n");
    }
}

int main(int argc, char * argv[])
{
    int result = 0;

    if(argc >= 2)
    {
        // glyr's control struct
        glyr_settings_t my_settings;

        // Init to the default settings
        glyr_init_settings( &my_settings);

        // Parse the commandline and store result in my_settings
        parse_commandline(argc, argv, &my_settings);
/*
	// or as library (just-an-example-of-course):
	glyr_setopt(&my_settings,GLYR_OPT_TYPE, GET_COVER);
	glyr_setopt(&my_settings,GLYR_OPT_ARTIST, "Equilibrium");
	glyr_setopt(&my_settings,GLYR_OPT_ALBUM, "Sagas");
	glyr_setopt(&my_settings,GLYR_OPT_UPDATE,true);
*/
        // Now execute...
        const char * filename = glyr_get(&my_settings);
        if(filename)
        {
            printf("%s\n",filename);
            free((char*)filename);
            filename = NULL;
        }
        else
        {
            result = 1;
        }

        // Clean memory
        glyr_destroy_settings( &my_settings);
    }
    else
    {
        usage();
    }
    return result;
}
