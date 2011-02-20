#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "core.h"
#include "types.h"
#include "stringop.h"

#include "ainfo/lastfm.h"

// Add your's here
plugin_t ainfo_providers[] =
{
//  full name       key  coloredname          use?   parser callback           geturl callback         free url?
    {"lastfm",      "l", "last"C_R"."C_"fm",  false,  {ainfo_lastfm_parse,    ainfo_lastfm_url,      false}},
    {"safe",        NULL,NULL,                false,  {NULL,                   NULL,                   false}},
    { NULL,         NULL, NULL,               false,  {NULL,                   NULL,                   false}},
};

plugin_t * glyr_get_ainfo_providers(void)
{
    return copy_table(ainfo_providers,sizeof(ainfo_providers));
}

static cache_list * ainfo_finalize(cache_list * result, glyr_settings_t * settings)
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

cache_list * get_ainfo(glyr_settings_t * settings)
{
    cache_list * result = NULL;
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
