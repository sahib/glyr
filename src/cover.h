#ifndef COVER_H
#define COVER_H

#include <stdbool.h>

char * get_cover(const char *artist, const char *album, const char *dir, char update, char max_parallel, const char *order);

#endif

