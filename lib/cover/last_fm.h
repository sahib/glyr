#ifndef LASTFM_H
#define LASTFM_H

#include "../core.h"

const char * cover_lastfm_url(GlyQuery * sets);
GlyCacheList * cover_lastfm_parse(cb_object *capo);

#endif
