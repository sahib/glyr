#ifndef A_ALLMUSIC_H
#define A_ALLMUSIC_H

#include "../core.h"

const char * ainfo_allmusic_url(GlyQuery *s);
GlyCacheList * ainfo_allmusic_parse(cb_object * capo);

#endif
