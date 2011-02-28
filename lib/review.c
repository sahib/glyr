#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "core.h"
#include "types.h"
#include "stringop.h"

#include "review/allmusic_com.h"

// Add your's here
GlyPlugin review_providers[] =
{
//  full name       key   coloredname          use?   parser callback           geturl callback        free url?
    {"allmusic",    "m",  C_"all"C_C"music",  false,  {review_allmusic_parse,  review_allmusic_url,    false}},
    {"safe",        NULL, NULL,               false,  {NULL,                   NULL,                   false}},
    { NULL,         NULL, NULL,               false,  {NULL,                   NULL,                   false}},
};

GlyPlugin * glyr_get_review_providers(void)
{
    return copy_table(review_providers,sizeof(review_providers));
}

static GlyCacheList * review_finalize(GlyCacheList * result, GlyQuery * settings)
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

GlyCacheList * get_review(GlyQuery * settings)
{
    GlyCacheList * result = NULL;
    if(settings && settings->artist)
    {
        result = register_and_execute(settings, review_finalize);
    }
    else
    {
        glyr_message(2,settings,stderr,C_R"* "C_"Artist is needed to find similiar artist (o rly?).\n");
    }
    return result;
}
