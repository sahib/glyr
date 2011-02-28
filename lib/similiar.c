#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "core.h"
#include "types.h"
#include "stringop.h"

#include "similiar/lastfm.h"

// Add your's here
GlyPlugin similiar_providers[] =
{
//  full name       key  coloredname          use?   parser callback           geturl callback         free url?
    {"lastfm",      "l", "last"C_R"."C_"fm",  false,  {similiar_lastfm_parse,  similiar_lastfm_url,    false}},
    {"safe",        NULL,NULL,                false,  {NULL,                   NULL,                   false}},
    { NULL,         NULL, NULL,               false,  {NULL,                   NULL,                   false}},
};

GlyPlugin * glyr_get_similiar_providers(void)
{
    return copy_table(similiar_providers,sizeof(similiar_providers));
}

static GlyCacheList * similiar_finalize(GlyCacheList * result, GlyQuery * settings)
{
    if(!result) return NULL;

    size_t i = 0;
    GlyCacheList * r_list = DL_new_lst();

    for(i = 0; i < result->size; i++)
    {
        // call user defined callback
        if(settings->callback.download)
            settings->callback.download(result->list[i],settings);

        DL_add_to_list(r_list,DL_copy(result->list[i]));
    }
    return r_list;
}

GlyCacheList * get_similiar(GlyQuery * settings)
{
    GlyCacheList * result = NULL;
    if(settings && settings->artist)
    {
        result = register_and_execute(settings, similiar_finalize);
    }
    else
    {
        glyr_message(2,settings,stderr,C_R"* "C_"Artist is needed to find similiar artist (o rly?).\n");
    }
    return result;
}
