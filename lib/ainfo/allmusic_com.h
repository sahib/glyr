#ifndef A_ALLMUSIC_H
#define A_ALLMUSIC_H

#include "../core.h"

const char * ainfo_allmusic_url(glyr_settings_t *s);
cache_list * ainfo_allmusic_parse(cb_object * capo);

#endif
