#ifndef L_LYRDB_H
#define L_LYRDB_H

#include "../core.h"

const char * lyrics_lyrdb_url(GlyQuery * settings);
GlyCacheList * lyrics_lyrdb_parse(cb_object *capo);

#endif
