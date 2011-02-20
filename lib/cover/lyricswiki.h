#ifndef LYRICSWIKI_H
#define LYRICSWIKI_H

#include "../core.h"

const char * cover_lyricswiki_url(glyr_settings_t * sets);
cache_list * cover_lyricswiki_parse(cb_object *capo);

#endif
