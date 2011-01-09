#ifndef COVER_H
#define COVER_H

#include <stdbool.h>

typedef struct cover_settings_t {
	const char * artist, * album,
                   * title,  * dir; 

	// only db
	// only dl 
	// int max_parallel
        // int max,miint max,min
        
 
} cover_settings_t; 

char * get_cover(const char *artist, const char *album, const char *dir, char update, char max_parallel, const char *order);

#endif

