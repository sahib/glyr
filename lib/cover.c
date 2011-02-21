#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "core.h"
#include "types.h"
#include "stringop.h"

//Include plugins:
#include "cover/last_fm.h"
#include "cover/coverhunt.h"
#include "cover/discogs.h"
#include "cover/amazon.h"
#include "cover/lyricswiki.h"
#include "cover/albumart.h"
#include "cover/google.h"

#define GOOGLE_COLOR C_B"g"C_R"o"C_Y"o"C_B"g"C_G"l"C_R"e"

// Add yours here.
plugin_t cover_providers[] =
{
    // Start of safe group
    {"last.fm",    "l", C_"last"C_R"."C_"fm", false, {cover_lastfm_parse,      cover_lastfm_url,     false}},
    {"albumart",   "b", C_R"albumart",        false, {cover_albumart_parse,    cover_albumart_url,   false}},
    {"safe",       NULL,   NULL,              false, {NULL,                    NULL,                 false}},
    {"lyricswiki", "w", C_C"lyricswiki",      false, {cover_lyricswiki_parse,  cover_lyricswiki_url, false}},
    {"coverhunt",  "c", C_G"coverhunt",       false, {cover_coverhunt_parse,   cover_coverhunt_url,  false}},
    {"unsafe",     NULL,  NULL,               false, {NULL,                    NULL,                 false}},
    {"discogs",    "d", C_"disc"C_Y"o"C_"gs", false, {cover_discogs_parse,     cover_discogs_url,    false}},
    {"amazon",     "a", C_Y"amazon",          false, {cover_amazon_parse,      cover_amazon_url,     false}},
    {"google",     "g", GOOGLE_COLOR,         false, {cover_google_parse,      cover_google_url,     true }},
    {"special",    NULL,  NULL,               false, {NULL,                    NULL,                 false}},
    { NULL,        NULL,  NULL,               false, {NULL,                    NULL,                 false}}
};

bool size_is_okay(int sZ, int min, int max)
{
    if((min == -1 && max == -1) ||
            (min == -1 && max >= sZ) ||
            (min <= sZ && max == -1) ||
            (min <= sZ && max >= sZ)  )
        return true;

    return false;
}

plugin_t * glyr_get_cover_providers(void)
{
    return copy_table(cover_providers,sizeof(cover_providers));
}

static cache_list * cover_callback(cb_object * capo)
{
    cache_list * ls = DL_new_lst();
    memCache_t * dl = DL_copy(capo->cache);
    if(dl)
    {
        dl->dsrc = strdup(capo->url);

        // call user defined callback
        if(capo->s->callback.download)
            capo->s->callback.download(dl,capo->s);

        DL_add_to_list(ls,dl);
    }
    return ls;
}

static cache_list * cover_finalize(cache_list * result, glyr_settings_t * settings)
{
    cache_list * dl_list = NULL;
    if(result)
    {
        if(settings->download)
        {
            cb_object  * urlplug_list = calloc(result->size+1,sizeof(cb_object));
            if(urlplug_list)
            {
                size_t i = 0;
                for(i = 0; i < result->size; i++)
                {
                    if(result->list[i]->data == NULL) puts("oh?");

                    plugin_init(&urlplug_list[i], result->list[i]->data, cover_callback, settings, NULL, NULL);
                }

                dl_list = invoke(urlplug_list,i,settings->parallel,settings->timeout * i, settings);
                free(urlplug_list);
            }
        }
        else
        {
            size_t i = 0;
            for( i = 0; i < result->size; i++)
            {
                if(result->list[i])
                {
                    if(!dl_list) dl_list = DL_new_lst();
                    memCache_t * r_copy = DL_copy(result->list[i]);
                    DL_add_to_list(dl_list,r_copy);
                }
            }
        }
    }
    return dl_list;
}

cache_list * get_cover(glyr_settings_t * settings)
{
    cache_list * res = NULL;
    if (settings && settings->artist && settings->album)
    {
        // validate size
        if(settings->cover.min_size <= 0)
            settings->cover.min_size = -1;

        if(settings->cover.max_size <= 0)
            settings->cover.max_size = -1;

        res = register_and_execute(settings, cover_finalize);
    }
    else
    {
        glyr_message(1,settings,stderr,C_R"[] "C_"%s is needed to download covers.\n",settings->artist ? "Album" : "Artist");
    }
    return res;
}
