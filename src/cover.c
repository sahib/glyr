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
    {"coverhunt",  "c", C_C"coverhunt",       false, {cover_coverhunt_parse,   cover_coverhunt_url,  false}},
    {"safe",       NULL,   NULL,              false, {NULL,                    NULL,                 false}},
    {"lyricswiki", "w", C_C"lyricswiki",      false, {cover_lyricswiki_parse,  cover_lyricswiki_url, false}},
    {"albumart",   "b", C_R"albumart",        false, {cover_albumart_parse,    cover_albumart_url,   false}},
    {"unsafe",     NULL,  NULL,               false, {NULL,                    NULL,                 false}},
    {"discogs",    "d", C_"disc"C_Y"o"C_"gs", false, {cover_discogs_parse,     cover_discogs_url,    false}},
    {"amazon",     "a", C_Y"amazon",          false, {cover_amazon_parse,      cover_amazon_url,     false}},
    {"google",     "g", GOOGLE_COLOR,         false, {cover_google_parse,      cover_google_url,     false}},
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

plugin_t *  glyr_get_cover_providers(void)
{
    return copy_table(cover_providers,sizeof(cover_providers));
}

static memCache_t ** cover_finalize(memCache_t * result, glyr_settings_t * settings)
{
    // result is guaranteed to be !NULL for usual
    // if result is NULL it will be the LAST call to this method
    if(!result)
    {
        return settings->cover.lst;
    }

    memCache_t * image = NULL;
    if((image = download_single(result->data,settings)) == NULL)
    {
        glyr_message(1,settings,stderr,C_R"??"C_" Found an apparently correct url, but unable to download <%s>\n",result->data);
        glyr_message(1,settings,stderr,C_R"??"C_" Please drop me a note, this might be a bug.\n");
        return settings->cover.lst;
    }

    if(settings->cover.lst == NULL)
        settings->cover.lst = DL_new_lst(settings->cover.number+1);

    settings->cover.lst[settings->cover.c_buf] = image;
    image->dsrc = strdup(result->data);

    if(settings->cover.number <= settings->cover.c_buf+1)
    {
        // mark end
        settings->cover.lst[settings->cover.c_buf+1] = NULL;

        // enoug covers. return
        return settings->cover.lst;
    }

    settings->cover.c_buf++;
    return NULL;
}

memCache_t ** get_cover(glyr_settings_t * settings)
{
    memCache_t ** res = NULL;
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
