#ifndef BOOKS_H
#define BOOKS_H

#include "types.h"

plugin_t   * glyr_get_books_providers(void);
cache_list * get_books(glyr_settings_t * settings);

#endif
