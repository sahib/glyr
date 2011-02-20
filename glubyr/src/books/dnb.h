#ifndef L_DNB_H
#define L_DNB_H

#include "../core.h"

const char * books_dnb_url(glyr_settings_t * settings);
cache_list * books_dnb_parse(cb_object * capo);

#endif
