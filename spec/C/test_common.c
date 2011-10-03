#include <stdlib.h>
#include <stdio.h>
#include <glib.h>

#include "../../lib/glyr.h"

void setup(GlyrQuery * q, GLYR_GET_TYPE type, int num)
{
    glyr_init();
    atexit(glyr_cleanup);

    glyr_query_init(q);
    glyr_opt_artist(q,"Equilibrium");
    glyr_opt_album(q, "Sagas");
    glyr_opt_title(q, "Wurzelbert");
    glyr_opt_type(q, type);
    glyr_opt_number(q,num);
    glyr_opt_verbosity(q,4);
}

//--------------------

GlyrQuery * setup_alloc(GLYR_GET_TYPE type, int num)
{
    GlyrQuery * new = g_malloc0(sizeof(GlyrQuery));
    setup(new,type,num);
    return new;
}

//--------------------

void unsetup(GlyrQuery * q, GlyrMemCache * list)
{
    glyr_free_list(list);
    glyr_query_destroy(q);
}

//--------------------

void init(void)
{
    glyr_init();
    atexit(glyr_cleanup);
}
