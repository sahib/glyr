#ifndef LYRICS_H
#define LYRICS_H

#include <stdbool.h>

typedef struct lyrics_settings_t 
{
	const char * artist, * album,
                   * title,  * dir; 
 
} lyrics_settings_t; 

char * get_lyrics(const char *artist, const char * album, const char *title, const char *dir, char update, char max_parallel, const char *order);

#endif

