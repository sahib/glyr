#ifndef L_LYRICSVIP_H
#define L_LYRICSVIP_H

#include "../types.h"

const char * lyrics_lyricsvip_url(glyr_settings_t * settings);
memCache_t * lyrics_lyricsvip_parse(cb_object *capo);

#endif
