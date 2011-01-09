#ifndef LASTFM_H
#define LASTFM_H

#include "../types.h"

const char * cover_lastfm_url(void);
memCache_t * cover_lastfm_parse(cb_object *capo);

#endif
