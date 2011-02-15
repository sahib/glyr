#ifndef LASTFM_H
#define LASTFM_H

#include "../core.h"

const char * cover_lastfm_url(glyr_settings_t * sets);
memCache_t * cover_lastfm_parse(cb_object *capo);

#endif
