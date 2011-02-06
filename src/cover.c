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

// Add yours here.
sk_pair_t cover_providers[] =
{
    // Start of safe group
    {"lyricswiki", "w", C_C"lyricswiki",    false, {cover_lyricswiki_parse,  cover_lyricswiki_url, false}},
    {"coverhunt",  "c", C_C"coverhunt",     false, {cover_coverhunt_parse,   cover_coverhunt_url,  false}},
    {"albumart",   "b", C_R"albumart",      false, {cover_albumart_parse,    cover_albumart_url,   false}},
    // End of safe group
    {"safe",       NULL,NULL,               false  },
    // Start of unsafe group
    {"discogs",    "d",C_"disc"C_Y"o"C_"gs",                      false, {cover_discogs_parse,     cover_discogs_url,    false}},
    {"last.fm",    "l",C_"last"C_R"."C_"fm",                      false, {cover_lastfm_parse,      cover_lastfm_url,     false}},
    {"amazon",     "a", C_Y"amazon",        false, {cover_amazon_parse,      cover_amazon_url,     false}},
    {"google",     "g",C_B"g"C_R"o"C_Y"o"C_B"g"C_G"l"C_R"e" ,     false, {cover_google_parse,      cover_google_url,     false}},
    // End of unsafe group
    {"unsafe",     NULL,NULL,  false}, // end of unsafe plugins
    // END
    { NULL,        NULL,NULL,  false}
};

sk_pair_t *  glyr_get_cover_providers(void)
{
    return cover_providers;
}

static const char * finalize(cb_object * plugin_list, size_t it,  const char *filename, glyr_settings_t * settings)
{
    // Now do the actual work
    memCache_t *result = invoke(plugin_list, it, COVER_MAX_PLUGIN, DEFAULT_TIMEOUT, DEFAULT_REDIRECTS, settings->artist, settings->album,"unused",NULL);

    if (result != NULL && result->data != NULL && filename)
    {
        // This is the final downdown
        memCache_t * image = download_single(result->data,1L);

        if (image == NULL)
        {
            fprintf(stderr,C_R"??"C_" Found an apparently correct url, but unable to download <%s>\n",result->data);
            fprintf(stderr,C_R"??"C_" Please drop me a note, this might be a bug.\n");
        }

        if(write_file(filename,image) == -1)
        {
            fprintf(stderr,C_R"Unable to write file to '"C_"%s"C_R"'\n"C_,filename);
        }

        DL_free(image);
        DL_free(result);
        result = NULL;

        return filename;
    }

    DL_free(result);
    return NULL;
}


char * get_cover(glyr_settings_t * settings, const char * filename)
{
    if (settings == NULL || settings->artist == NULL || settings->album  == NULL)
    {
        fprintf(stderr,C_R"(!) "C_"<%s> is empty, but it is needed for coverDL!\n",settings->artist ? "album":"artist" );
        return NULL;
    }

    if(register_and_execute(settings, filename, finalize) != NULL)
    {
        return strdup(filename);
    }
    else
    {
        return NULL;
    }
}
