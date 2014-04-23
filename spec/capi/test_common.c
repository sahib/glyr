/***********************************************************
 * This file is part of glyr
 * + a command-line tool and library to download various sort of musicrelated metadata.
 * + Copyright (C) [2011-2012]  [Christopher Pahl]
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
#include <stdio.h>
#include <glib.h>

#include "../../lib/glyr.h"

void setup (GlyrQuery * q, GLYR_GET_TYPE type, int num)
{
    glyr_query_init (q);
    glyr_opt_artist (q,"Equilibrium");
    glyr_opt_album (q, "Sagas");
    glyr_opt_title (q, "Wurzelbert");
    glyr_opt_type (q, type);
    glyr_opt_number (q,num);
    glyr_opt_verbosity (q,4);
}

//--------------------

GlyrQuery * setup_alloc (GLYR_GET_TYPE type, int num)
{
    GlyrQuery * new = g_malloc0 (sizeof (GlyrQuery) );
    setup (new,type,num);
    return new;
}

//--------------------

void unsetup (GlyrQuery * q, GlyrMemCache * list)
{
    glyr_free_list (list);
    glyr_query_destroy (q);
}

//--------------------

void init (void)
{
    glyr_init();
    atexit (glyr_cleanup);
}
