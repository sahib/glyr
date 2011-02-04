#include <stdlib.h>
#include <stdio.h>
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


int main(int argc, char * argv[])
{
    if(argc >= 2)
    {
        // glyr's control struct
        glyr_settings_t my_settings;

        // Init to the default settings
        glyr_init_settings( &my_settings);

        // Parse the commandline
        if(glyr_parse_commandline(argc, argv, &my_settings) == false)
            usage();

        // Now execute..
        const char * filename = glyr_get(&my_settings);
        if(filename)
        {
            puts(filename);
            free((char*)filename);
            filename =NULL;
        }

        // Clean memory
        glyr_destroy_settings( &my_settings);
    }
    else
    {
        usage();
    }
    return 0;
}
