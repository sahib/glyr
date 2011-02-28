#ifndef LYRICS_H
#define LYRICS_H

#include "types.h"

GlyPlugin   * glyr_get_lyric_providers(void);
GlyCacheList * get_lyrics(GlyQuery * settings);

#endif

