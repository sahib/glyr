#ifndef L_LYRICSWIKI_H
#define L_LYRICSWIKI_H

#include "../types.h"

const char * lyrics_lyricswiki_url(void);
memCache_t * lyrics_lyricswiki_parse(cb_object *capo);

#endif
