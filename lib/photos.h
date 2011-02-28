#ifndef PHOTOS_H
#define PHOTOS_H

#include "types.h"

GlyPlugin   * glyr_get_photo_providers(void);
GlyCacheList * get_photos(GlyQuery * settings);

#endif
