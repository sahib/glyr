#ifndef GLYR_H
#define GLYR_H

// va_list for glyr_setopt()
#include <stdarg.h>

// all structs used by glyr are here
#include "types.h"

enum GLYR_OPTIONS
{
    GLYRO_TYPE,
    GLYRO_ARTIST,
    GLYRO_ALBUM,
    GLYRO_TITLE,
    GLYRO_CMINSIZE,
    GLYRO_CMAXSIZE,
    GLYRO_PARALLEL,
    GLYRO_TIMEOUT,
    GLYRO_REDIRECTS,
    GLYRO_NUMBER,
    GLYRO_VERBOSITY,
    GLYRO_COLOR,
    GLYRO_INFO_AT,
    GLYRO_FROM,
    GLYRO_PLUGMAX,
    GLYRO_LANG,
    GLYRO_DOWNLOAD
};
enum GLYR_ERROR
{
    GLYRE_OK,           // everything is fine
    GLYRE_BAD_OPTION,   // you passed a bad option to glyr_setopt()
    GLYRE_BAD_VALUE,    // Invalid value in va_list
    GLYRE_EMPTY_STRUCT, // you passed an empty struct to glyr_setopt()
    GLYRE_NO_PROVIDER,  // setttings->provider == NULL
    GLYRE_UNKNOWN_GET   // settings->type is not valid
};
#ifdef __cplusplus
extern "C"
{
#endif
    // the actual main of glyr
    cache_list * glyr_get(glyr_settings_t * settings, int * error);

    // Initialize settings struct to sane defaults
    void glyr_init_settings(glyr_settings_t * glyrs);

    // free all memory that may been still allocated in struct
    void glyr_destroy_settings(glyr_settings_t * sets);

    // set a specififc option
    int glyr_setopt(glyr_settings_t * s, int option, ...);

    // free results
    void glyr_free_list(cache_list * lst);
    void glyr_free_cache(memCache_t * c);
    void glyr_add_to_list(cache_list * l, memCache_t * c);

    // Set a callback that is executed always whenever a cache is 'ready' (i.e. will appear in cache_list)
    void glyr_set_dl_callback(glyr_settings_t * settings, void (*dl_cb)(memCache_t *, glyr_settings_t *), void * userp);

    // return library version
    const char * glyr_version(void);

    // print method that works with verbosity
    int glyr_message(int v, glyr_settings_t * s, FILE * stream, const char * fmt, ...);

    // get information about available plugins
    plugin_t * glyr_get_provider_by_id(int ID);

#ifdef _cplusplus
}
#endif

#endif
