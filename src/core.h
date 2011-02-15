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
    memCache_t * (* parser_callback) (struct cb_object *);

    // What url to download before the callback is called
    char *url;

    // Storage of the --of argument
    const char ** info;

    // Custom pointer
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

void plugin_init(cb_object *ref, const char *url, memCache_t*(callback)(cb_object*), glyr_settings_t * s, const char *name, void * custom);
void glyr_init_settings(glyr_settings_t * glyrs);

memCache_t ** register_and_execute(glyr_settings_t * settings, memCache_t ** (* finalizer) (memCache_t *, glyr_settings_t *));

memCache_t * invoke(cb_object *oblist, long CNT, long parallel, long timeout, glyr_settings_t * s);
memCache_t * download_single(const char* url, glyr_settings_t * s);

memCache_t *  DL_init(void);
memCache_t ** DL_new_lst(int n);
void          DL_free(memCache_t *cache);
void          DL_free_lst(memCache_t ** lst, glyr_settings_t *s);

plugin_t * copy_table(const plugin_t * o, size_t size);

int glyr_message(int v, glyr_settings_t * s, FILE * stream, const char * fmt, ...);

#endif
