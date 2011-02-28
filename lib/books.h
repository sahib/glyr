#ifndef BOOKS_H
#define BOOKS_H

#include "types.h"

GlyPlugin   * glyr_get_books_providers(void);
GlyCacheList * get_books(GlyQuery * settings);

#endif
