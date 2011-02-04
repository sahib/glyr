#ifndef CORE_H
#define CORE_H

#include "types.h"

void plugin_init(cb_object *ref, const char *url, memCache_t*(callback)(cb_object*), int min, int max, const char *name);
void DL_free(memCache_t *cache);

memCache_t * invoke(cb_object *oblist, long CNT, long parallel, const char *artist, const char* album, const char *title);
memCache_t * download_single(const char* url, long redirects);
memCache_t * DL_init(void);

int write_file(const char *path, memCache_t *data);
int sk_is_in(sk_pair_t * arr, const char * string);
void glyr_init_settings(glyr_settings_t * glyrs);

const char * register_and_execute(glyr_settings_t * settings, const char * filename, const char * (* finalizer) (cb_object *, size_t it, const char *, glyr_settings_t *));
#endif
