#ifndef L_LYRDB_H
#define L_LYRDB_H

#include "../core.h"

const char * lyrics_lyrdb_url(glyr_settings_t * settings);
cache_list * lyrics_lyrdb_parse(cb_object *capo);

#endif
