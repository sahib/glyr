#ifndef LYRICSWIKI_H
#define LYRICSWIKI_H

#include "../types.h"

const char * cover_lyricswiki_url(glyr_settings_t * sets);
memCache_t * cover_lyricswiki_parse(cb_object *capo);

#endif
