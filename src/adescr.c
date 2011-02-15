#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "core.h"
#include "types.h"
#include "stringop.h"

#include "adescr/lastfm.h"

// Add your's here
plugin_t adescr_providers[] =
{
//  full name       key  coloredname          use?   parser callback           geturl callback         free url?
    {"lastfm",      "l", "last"C_R"."C_"fm",  false,  {adescr_lastfm_parse,    adescr_lastfm_url,      false}},
    {"safe",        NULL,NULL,                false,  {NULL,                   NULL,                   false}},
    { NULL,         NULL, NULL,               false,  {NULL,                   NULL,                   false}},
};

plugin_t * glyr_get_adescr_providers(void)
{
    return copy_table(adescr_providers,sizeof(adescr_providers));
}

static memCache_t ** adescr_finalize(memCache_t * result, glyr_settings_t * settings)
{
    if(!result) return NULL;

    const char * split_mark = "__split__";
    char * short_split = strstr(result->data,split_mark);

    memCache_t ** lst = NULL;

    if(short_split)
    {
	// copy summary
	char * short_descr = copy_value(result->data,short_split);

	// allocate long description buf
	size_t buf_size = result->size - (short_split - result->data);
	char * long_descr  = calloc(buf_size + 1, sizeof(char));

	// copy end of buf
	strncpy(long_descr,short_split + strlen(split_mark), buf_size - strlen(split_mark));
	
	// prepare return 
	lst = DL_new_lst(2);

	lst[0] = DL_init();
	lst[1] = DL_init();

	// pass short
	lst[0]->data = short_descr;
	lst[0]->size = short_split - result->data;
	lst[0]->dsrc = strdup(result->dsrc);

	lst[1]->data = long_descr;
	lst[1]->size = buf_size;
	lst[1]->dsrc = strdup(result->dsrc);
    }
    else
    {
	glyr_message(1,settings,stderr,"[Internal]: Unable to find splitmark. Returning nothing\n");
    }

    return lst;
}

memCache_t ** get_adescr(glyr_settings_t * settings)
{
    memCache_t ** result = NULL;
    if(settings && settings->artist)
    {
        result = register_and_execute(settings, adescr_finalize);
    }
    else
    {
        glyr_message(2,settings,stderr,C_R":: "C_"Artist is needed to download artist description.\n");
    }
    return result;
}
