#ifndef L_KIT_H
#define L_KIT_H

#include "../core.h"

const char * books_kit_url(GlyQuery * settings);
GlyCacheList * books_kit_parse(cb_object * capo);

#endif
