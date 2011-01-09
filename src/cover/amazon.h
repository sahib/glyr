#ifndef AMAZON_H
#define AMAZON_H

#include "../types.h"

const char * cover_amazon_url_lng(char LANG_ID);
memCache_t * cover_amazon_parse(cb_object *capo);

#endif
