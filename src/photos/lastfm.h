#ifndef P_LASTFM_H
#define P_LASTFM_H

#include "../types.h"

const char * photos_lastfm_url(glyr_settings_t * settings);
memCache_t * photos_lastfm_parse(cb_object * capo);

#endif
