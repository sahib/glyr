#ifndef G_TRANSLATE_H
#define G_TRANSLATE_H

// gtrans method family

#include "types.h"
/*  Takes the cache struct to_translate and translate the text according to the setting stored in the GlyQuery s    *
    It will be translated IF to_translate is not an image (is_image == false) and the targetlang != sourcelang,     *
    and both languages are valid. Note that the usage of these functions is restricted by google to a certain limit *
    being at the moment at 100,000 chars per day and IP.
    The buffer is directly modified so make copies before if you need.
*/
void  Gly_gtrans_translate(GlyQuery * s, GlyMemCache * to_translate);

/* Ask google what language the snippet given as const char is written in *
   Optionally you can give a float pointer where the probability of the   *
   result being correct is stored in, (from 0.0 to 1.0), long snippets    *
   usually give better results. The result is returned as newly allocated *
   char * - free it.
*/
char *  Gly_gtrans_lookup(GlyQuery * s, const char * snippet, float * correctness);

/* Queries google for a list of all supported languages *
   Returned as a list of char pointers, free both the   *
   single strings and the return value.                 * 
   If you only want to know all languages:              *
   'glyrc gtrans list' does the same.                   *
*/
char ** Gly_gtrans_list(GlyQuery * s);

#endif
