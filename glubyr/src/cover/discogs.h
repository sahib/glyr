#ifndef DISCOGS_H
#define DISCOGS_H

#include "../core.h"

const char * cover_discogs_url(glyr_settings_t * sets);
cache_list * cover_discogs_parse(cb_object *capo);

#endif
