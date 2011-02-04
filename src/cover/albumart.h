#ifndef C_ALBUMART_H
#define C_ALBUMART_H

#include "../types.h"

const char * cover_albumart_url(glyr_settings_t * sets);
memCache_t * cover_albumart_parse(cb_object * capo);

#endif
