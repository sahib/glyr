#ifndef L_MAGISTRIX_H
#define L_MAGISTRIX_H

#include "../core.h"

const char * lyrics_magistrix_url(GlyQuery * settings);
GlyCacheList * lyrics_magistrix_parse(cb_object *capo);

#endif
