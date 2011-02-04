#ifndef GLYR_H
#define GLYR_H

#include <stdbool.h>
#include "types.h"

const char * glyr_get(glyr_settings_t * settings);
bool glyr_parse_commandline(int argc, char * const * argv, glyr_settings_t * glyrs);

void glyr_destroy_settings(glyr_settings_t * sets);
void glyr_init_settings(glyr_settings_t * glyrs);
#endif
