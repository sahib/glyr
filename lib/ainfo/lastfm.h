#ifndef A_LASTFM_H
#define A_LASTFM_H

#include "../core.h"

const char * ainfo_lastfm_url(GlyQuery * s);
GlyCacheList * ainfo_lastfm_parse(cb_object * capo );

#endif
