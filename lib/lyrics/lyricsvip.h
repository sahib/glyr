#ifndef L_LYRICSVIP_H
#define L_LYRICSVIP_H

#include "../core.h"

const char * lyrics_lyricsvip_url(GlyQuery * settings);
GlyCacheList * lyrics_lyricsvip_parse(cb_object *capo);

#endif
