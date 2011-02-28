#ifndef LYRICSWIKI_H
#define LYRICSWIKI_H

#include "../core.h"

const char * cover_lyricswiki_url(GlyQuery * sets);
GlyCacheList * cover_lyricswiki_parse(cb_object *capo);

#endif
