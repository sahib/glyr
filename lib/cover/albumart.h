#ifndef C_ALBUMART_H
#define C_ALBUMART_H

#include "../core.h"

const char * cover_albumart_url(GlyQuery * sets);
GlyCacheList * cover_albumart_parse(cb_object * capo);

#endif
