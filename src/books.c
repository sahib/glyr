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
    {"safe",        NULL, NULL,         false },
    { NULL,         NULL, NULL,         false },
};

plugin_t * glyr_get_books_providers(void)
{
    return copy_table(books_providers,sizeof(books_providers));
}

static memCache_t ** books_finalize(memCache_t * result, glyr_settings_t * settings)
{
    if(!result) return NULL;

    memCache_t * res = DL_init();
    res->data = beautify_lyrics(result->data);
    res->size = strlen(res->data);

   // printf("%s\n",res->data);

    memCache_t ** lst = DL_new_lst(1);
    lst[0] = res;
    return lst;
}

memCache_t ** get_books(glyr_settings_t * settings)
{
    if(settings && settings->artist)
    {
        return register_and_execute(settings,books_finalize);
    }
    return NULL;
}

