#ifndef C_ALLMUSIC_H
#define C_ALLMUSIC_H

#include "../core.h"

const char * cover_allmusic_url(glyr_settings_t *s);
cache_list * cover_allmusic_parse(cb_object * capo);

#endif
