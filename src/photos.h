
#ifndef PHOTOS_H
#define PHOTOS_H

#include "types.h"

sk_pair_t * glyr_get_photo_providers(void);
char * get_photos(glyr_settings_t * settings, const char * filename);

#endif
