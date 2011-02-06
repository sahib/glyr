#ifndef CORE_H
#define CORE_H

#include "types.h"

void plugin_init(cb_object *ref, const char *url, memCache_t*(callback)(cb_object*), int min, int max, const char *name);
void glyr_init_settings(glyr_settings_t * glyrs);
void DL_free(memCache_t *cache);

const char * register_and_execute(glyr_settings_t * settings, const char * filename, const char * (* finalizer) (cb_object *, size_t it, const char *, glyr_settings_t *));
memCache_t * invoke(cb_object *oblist, long CNT, long parallel, long timeout, long redirects, const char *artist, const char* album, const char *title, const char * status);
memCache_t * download_single(const char* url, long redirects);
memCache_t * DL_init(void);

int write_file(const char *path, memCache_t *data);

#endif
