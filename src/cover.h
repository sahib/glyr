#ifndef COVER_H
#define COVER_H

#include <stdbool.h>
#include "types.h"

plugin_t *  glyr_get_cover_providers(void);
char * get_cover(glyr_settings_t * settings);
bool size_is_okay(int sZ, int min, int max);

#endif

