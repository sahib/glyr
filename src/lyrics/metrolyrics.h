#ifndef L_METROLYRICS_H
#define L_METROLYRICS_H

#include "../core.h"

const char * lyrics_metrolyrics_url(glyr_settings_t * settings);
memCache_t * lyrics_metrolyrics_parse(cb_object *capo);

#endif
