#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "core.h"
#include "types.h"
#include "stringop.h"

#include "books.h"
#include "books/dnb.h"
#include "books/kit.h"

// Add your's here
plugin_t books_providers[] =
{
//  full name       key  coloredname    use?    parser callback           geturl callback         free url?
    {"kit",         "k", "kit",         false, {books_kit_parse,          books_kit_url,          false}},
    {"dnb",         "n", "dnb",         false, {books_dnb_parse,          books_dnb_url,          false}},
    {"safe",        NULL,NULL,          false },
    { NULL,         NULL, NULL,         false },
};

plugin_t * glyr_get_books_providers(void)
{
    return copy_table(books_providers,sizeof(books_providers));
}

#define multi_fprintf(...) fprintf(handle, __VA_ARGS__); fprintf(stdout, __VA_ARGS__)

bool write_text(const char * path, const char * text)
{
    if(path)
    {
        FILE * handle = fopen(path,"w");
        if(handle)
        {
            multi_fprintf("\n%s\n",text);
            fclose(handle);
            return true;
        }
    }
    return false;
}

// let this be only a local phenomena
#undef multi_fprintf

static const char * books_finalize(memCache_t * result, glyr_settings_t * settings, const char * filename)
{
        char * blyr = beautify_lyrics(result->data);

        if(write_text(filename,blyr) == false)
        {
            fprintf(stderr,C_R"Unable to write bookinfo '"C_"%s"C_R"'\n"C_,filename);
        }

        if(blyr)
            free(blyr);

        return filename;

    return NULL;
}

char * get_books(glyr_settings_t * settings)
{
    if(settings && settings->artist)
    {
    	char * filename = strdup_printf("%s/%s.book",settings->save_path,settings->artist);
	if(filename)
	{
    		return (char*)register_and_execute(settings, filename, books_finalize);
	}
    }
    return NULL;
}

