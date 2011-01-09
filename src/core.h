#ifndef CORE_H
#define CORE_H

#include "types.h"

void plugin_init(cb_object *ref, const char *url, memCache_t*(callback)(cb_object*), int min, int max, const char *name);
void DL_free(memCache_t *cache);

memCache_t * invoke(cb_object *oblist, long CNT, long parallel, const char *artist, const char* album, const char *title);
memCache_t * download_single(const char* url, long redirects);
memCache_t * DL_init(void);

int write_file(const char *path, memCache_t *data);

#endif
