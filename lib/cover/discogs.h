#ifndef DISCOGS_H
#define DISCOGS_H

#include "../core.h"

const char * cover_discogs_url(GlyQuery * sets);
GlyCacheList * cover_discogs_parse(cb_object *capo);

#endif
