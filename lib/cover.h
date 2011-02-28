#ifndef COVER_H
#define COVER_H

#include <stdbool.h>
#include "types.h"

GlyPlugin   * glyr_get_cover_providers(void);
GlyCacheList * get_cover(GlyQuery * settings);
bool size_is_okay(int sZ, int min, int max);

#endif

