#ifndef CORE_H
#define CORE_H

#include "types.h"

// Internal calback object, used for cover, lyrics and other
// This is only used inside the core and the plugins
// Other parts of the program shall not use this struct
// memCache_t is what you're searching
// It models the data that one plugin needs.
typedef struct cb_object
{
    // What callback to call
    cache_list * (* parser_callback) (struct cb_object *);

    // What url to download before the callback is called
    char *url;

    // Storage of the --of argument
    const char ** info;

    // Custom pointer used to pass
    // unspecified data to finalize()
    void * custom;

    // pointer to settings struct (artist,album,etc)
    glyr_settings_t * s;

    //Plugin name - only important for verbose output
    const char * name;

    // What curl handle this is attached
    CURL *handle;

    // internal cache attached to this url
    memCache_t *cache;

} cb_object;


void plugin_init(cb_object *ref, const char *url, cache_list * (callback)(cb_object*), glyr_settings_t * s, const char *name, void * custom);
void glyr_init_settings(glyr_settings_t * glyrs);

cache_list * register_and_execute(glyr_settings_t * settings, cache_list * (* finalizer) (cache_list *, glyr_settings_t *));

cache_list * invoke(cb_object *oblist, long CNT, long parallel, long timeout, glyr_settings_t * s);
memCache_t * download_single(const char* url, glyr_settings_t * s);

memCache_t * DL_init(void);
cache_list * DL_new_lst(void);
void DL_free(memCache_t *cache);
void DL_free_lst(cache_list * c);
memCache_t * DL_copy(memCache_t * src);
void DL_add_to_list(cache_list * l, memCache_t * c);
void DL_free_container(cache_list * c);

plugin_t * copy_table(const plugin_t * o, size_t size);

int glyr_message(int v, glyr_settings_t * s, FILE * stream, const char * fmt, ...);

#endif
