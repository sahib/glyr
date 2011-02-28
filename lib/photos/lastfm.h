#ifndef P_LASTFM_H
#define P_LASTFM_H

#include "../core.h"
#include "../core.h"

const char * photos_lastfm_url(GlyQuery * settings);
GlyCacheList * photos_lastfm_parse(cb_object * capo);

#endif
