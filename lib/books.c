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

static cache_list * books_finalize(cache_list * result, glyr_settings_t * settings)
{
    if(!result) return NULL;

    size_t i = 0;
    cache_list * r_list = DL_new_lst();

    for(i = 0; i < result->size; i++)
    {
        DL_add_to_list(r_list,DL_copy(result->list[i]));
    }
    return r_list;
}

cache_list * get_books(glyr_settings_t * settings)
{
    if(settings && settings->artist)
    {
        return register_and_execute(settings,books_finalize);
    }
    return NULL;
}

