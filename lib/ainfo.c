#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "core.h"
#include "types.h"
#include "stringop.h"

#include "ainfo/lastfm.h"
#include "ainfo/allmusic_com.h"

// Add your's here
GlyPlugin ainfo_providers[] =
{
//  full name       key  coloredname          use?   parser callback           geturl callback         free url?
    {"lastfm",      "l", "last"C_R"."C_"fm",  false,  {ainfo_lastfm_parse,     ainfo_lastfm_url,       true }},
    {"safe",        NULL,NULL,                false,  {NULL,                   NULL,                   false}},
    {"allmusic",    "m", C_"all"C_C"music",   false,  {ainfo_allmusic_parse,   ainfo_allmusic_url,     false}},
    {"unsafe",      NULL,NULL,                false,  {NULL,                   NULL,                   false}},
    { NULL,         NULL, NULL,               false,  {NULL,                   NULL,                   false}},
};

GlyPlugin * glyr_get_ainfo_providers(void)
{
    return copy_table(ainfo_providers,sizeof(ainfo_providers));
}

static GlyCacheList * ainfo_finalize(GlyCacheList * result, GlyQuery * settings)
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

GlyCacheList * get_ainfo(GlyQuery * settings)
{
    GlyCacheList * result = NULL;
    if(settings && settings->artist)
    {
        result = register_and_execute(settings, ainfo_finalize);
    }
    else
    {
        glyr_message(2,settings,stderr,C_R":: "C_"Artist is needed to download artist description.\n");
    }
    return result;
}
