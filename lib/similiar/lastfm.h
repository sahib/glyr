#ifndef S_LASTFM_H
#define S_LASTFM_H

#include "../core.h"

const char * similiar_lastfm_url(GlyQuery * s);
GlyCacheList * similiar_lastfm_parse(cb_object * capo);

#endif
