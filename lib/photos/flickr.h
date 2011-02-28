#ifndef P_FLICKR_H
#define P_FLICKR_H

#include "../core.h"

const char * photos_flickr_url(GlyQuery * settings);
GlyCacheList * photos_flickr_parse(cb_object * capo);

#endif
