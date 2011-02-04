#ifndef COVER_H
#define COVER_H

#include <stdbool.h>
#include "types.h"

sk_pair_t *  glyr_get_cover_providers(void);
char * get_cover(glyr_settings_t * settings, const char * filename);

#endif

