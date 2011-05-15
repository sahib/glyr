#ifndef G_TRANSLATE_H
#define G_TRANSLATE_H

#include "types.h"
void Gly_translate_text(GlyQuery * s, GlyMemCache * to_translate);
char * Gly_lookup_language(GlyQuery * s, const char * snippet, float * correctness);
char ** Gly_list_supported_languages(GlyQuery * s);

#endif
