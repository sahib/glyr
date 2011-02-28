#ifndef L_LYRICSWIKI_H
#define L_LYRICSWIKI_H

#include "../core.h"

const char * lyrics_lyricswiki_url(GlyQuery * settings);
GlyCacheList * lyrics_lyricswiki_parse(cb_object *capo);

#endif
