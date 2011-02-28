#ifndef L_DIRECTLYRICS_H
#define L_DIRECTLYRICS_H

#include "../core.h"

const char * lyrics_directlyrics_url(GlyQuery * settings);
GlyCacheList * lyrics_directlyrics_parse(cb_object *capo);

#endif
