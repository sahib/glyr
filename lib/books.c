/***********************************************************
* This file is part of glyr
* + a commnadline tool and library to download various sort of musicrelated metadata.
* + Copyright (C) [2011]  [Christopher Pahl]
* + Hosted at: https://github.com/sahib/glyr
*
* glyr is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* glyr is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with glyr. If not, see <http://www.gnu.org/licenses/>.
**************************************************************/
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
GlyPlugin books_providers[] =
{
//  full name       key  coloredname    use?    parser callback           geturl callback         free url?
    {"kit",         "k", "kit",         false, {books_kit_parse,          books_kit_url,          false}},
    {"dnb",         "n", "dnb",         false, {books_dnb_parse,          books_dnb_url,          false}},
    {"safe",        NULL, NULL,         false },
    { NULL,         NULL, NULL,         false },
};

GlyPlugin * glyr_get_books_providers(void)
{
    return copy_table(books_providers,sizeof(books_providers));
}

static GlyCacheList * books_finalize(GlyCacheList * result, GlyQuery * settings)
{
    if(!result) return NULL;

    size_t i = 0;
    GlyCacheList * r_list = DL_new_lst();

    for(i = 0; i < result->size; i++)
    {
        DL_add_to_list(r_list,DL_copy(result->list[i]));
    }
    return r_list;
}

GlyCacheList * get_books(GlyQuery * settings)
{
    if(settings && settings->artist)
    {
        return register_and_execute(settings,books_finalize);
    }
    return NULL;
}

