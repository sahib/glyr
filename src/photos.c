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

static memCache_t * photo_callback(cb_object * capo)
{
    size_t i = capo->s->photos.offset;
    memCache_t ** lst = capo->custom;

    // find next empty place
    // this function shall not be called
    // more often than the lst is big
    // if a overflow occures it's because
    // photo_finalize did sth. weird.
    for(; lst[i]->data; i++);

    // Set!
    lst[i]->data = malloc(capo->cache->size+1);
    memcpy(lst[i]->data, capo->cache->data, capo->cache->size);
    lst[i]->size = capo->cache->size;

    // return null so others 'plugins' are called
    return NULL;
}

static memCache_t ** photo_finalize(memCache_t * result, glyr_settings_t * settings)
{
    size_t i, urlc = 0;

    // no need for cleanup
    if(!result)
        return NULL;
    if(!result->size)
        return NULL;
    if(result->size <= settings->photos.offset)
    {
	glyr_message(1,settings,stderr,C_R"[]"C_" photo_offset is equal or higher than the number of available images.\n");
    }


    // list of buffers
    memCache_t ** cache_lst = DL_new_lst(result->size);

    // Every url is treated as a plugin, and downloaded in parallel
    // This is a little bit of missuse, but who cares ;-)
    cb_object * url_list    = calloc(result->size, sizeof(cb_object));

    char * l_url = NULL;
    char * c_url = result->data+1;

    for(i = settings->photos.offset; i < result->size && i < settings->photos.number; i++)
    {
        l_url = c_url;
        c_url = strstr(l_url,"\n");

        if(l_url)
        {
            size_t  url_len;
            if(c_url)
                url_len = c_url - l_url;
            else
                url_len = strlen(l_url);

            if(url_len > 0)
            {
                char * url = malloc(url_len+1);

                if(url)
                {
                    // copy url
                    strncpy(url,l_url,url_len);
                    url[url_len] = '\0';

                    cache_lst[i] = DL_init();
                    cache_lst[i]->dsrc = url;

                    // Init "plugin" (this is actually a missuse of plugin_init())
                    // Init only when $artist_photo_$i does not exit yet, or if update is desired
                    plugin_init(&url_list[urlc++],url,photo_callback,settings,NULL,cache_lst);
                }
            }
        }
        c_url++;
    }

    // This shall always return NULL, so no extra free is needed
    // The timeout is result->size as big because we need to download $result->size images
    // Arg  6,7,8 are not used also, they may not be empyt though
    invoke(url_list, urlc, settings->parallel, settings->timeout * urlc, settings);

    // free all the rest
    if(url_list)
        free(url_list);

    url_list=NULL;
    return cache_lst;
}


memCache_t ** get_photos(glyr_settings_t * settings)
{
    if (settings && settings->artist)
        return register_and_execute(settings,photo_finalize);
    else
	glyr_message(2,settings,stderr,C_R"[]"C_" Artist is needed to download artist-related photos!\n");

    return NULL;
}
