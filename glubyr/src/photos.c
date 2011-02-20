#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "photos.h"
#include "core.h"
#include "types.h"
#include "stringop.h"

#include "photos/flickr.h"
#include "photos/lastfm.h"

plugin_t photos_providers[] =
{
    {"lastfm", "l",  C_"last"C_R"."C_"fm",  false, {photos_lastfm_parse, photos_lastfm_url, false}},
    {"safe",   NULL, NULL,                  false, {NULL,              NULL,                false}},
    {"flickr", "f",  C_C"flick"C_R"r",      false, {photos_flickr_parse, photos_flickr_url, true }},
    {"unsafe", NULL, NULL,                  false, {NULL,              NULL,                false}},
    {NULL,     NULL, NULL,                  false, {NULL,              NULL,                false}}
};

plugin_t * glyr_get_photo_providers(void)
{
    return copy_table(photos_providers,sizeof(photos_providers));
}

static cache_list * cover_callback(cb_object * capo)
{
	cache_list * ls = DL_new_lst();
	memCache_t * dl = DL_copy(capo->cache);
	if(dl)
	{
		dl->dsrc = strdup(capo->url);
		DL_add_to_list(ls,dl);	
	}
	return ls;
}

static cache_list * photo_finalize(cache_list * result, glyr_settings_t * settings)
{
    cache_list * dl_list = NULL;
    if(result)
    {
	cb_object  * urlplug_list = calloc(result->size+1,sizeof(cb_object));
	if(urlplug_list)
	{
		size_t i = 0;
		for(i = 0; i < result->size; i++)
		{
			if(result->list[i] && result->list[i]->data)
			{
				plugin_init(&urlplug_list[i], result->list[i]->data, cover_callback, settings, NULL, NULL);
				free(result->list[i]->data);
			}
		}
printf("\n%d timeout\n", settings->timeout);
		dl_list = invoke(urlplug_list,i,settings->parallel,settings->timeout * i, settings);
		free(urlplug_list);
	}
    }
    return dl_list;
}


cache_list * get_photos(glyr_settings_t * settings)
{
    if (settings && settings->artist)
        return register_and_execute(settings,photo_finalize);
    else
	glyr_message(2,settings,stderr,C_R"[]"C_" Artist is needed to download artist-related photos!\n");

    return NULL;
}
