#ifndef R_ALLMUSIC_H
#define R_ALLMUSIC_H

#include "../core.h"

const char *   review_allmusic_url(GlyQuery * s);
GlyCacheList * review_allmusic_parse(cb_object * capo);

#endif
