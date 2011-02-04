#ifndef AMAZON_H
#define AMAZON_H

#include "../types.h"

const char * cover_amazon_url(glyr_settings_t * sets);
memCache_t * cover_amazon_parse(cb_object *capo);

#endif
