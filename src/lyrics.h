#ifndef LYRICS_H
#define LYRICS_H

#include "types.h"

plugin_t * glyr_get_lyric_providers(void);
char * get_lyrics(glyr_settings_t * settings);

#endif

