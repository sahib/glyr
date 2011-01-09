#ifndef DISCOGS_H
#define DISCOGS_H

#include "../types.h"

const char * cover_discogs_url(void);
memCache_t * cover_discogs_parse(cb_object *capo);

#endif
