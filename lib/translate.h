/**
* @file translate.h
* @brief (do not use yet) Methods to interact directly with google translator
*
* Do not use those yet. They should work fine, but are limited to 100000 chars/day(/IP?)
*
* @author Christopher Pahl
* @version 0.6
* @date 2011-06-14
*/
#ifndef G_TRANSLATE_H
#define G_TRANSLATE_H

#include "types.h"
/**
* @brief Initiate translating
*
* @param s The GlyQuery needed to determine the source/target language
* @param to_translate Cache that gets modified with the translation.
*
*  Takes the cache struct to_translate and translate the text according to the setting stored in the GlyQuery s \n
*  It will be translated IF to_translate is not an image (is_image == false) and the targetlang != sourcelang,   \n
*  and both languages are valid. Note that the usage of these functions is restricted by google to a certain limit \n
*  being at the moment at 100,000 chars per day and IP.\n
*  The buffer is directly modified so make copies before if you need.\n
*/
void  Gly_gtrans_translate(GlyQuery * s, GlyMemCache * to_translate);

/**
* @brief Lookup the language a certain snippet may be in
*
* @param s The GlyQuery needed to determine the source/target language\n
* @param snippet a short snippet (less than 500 signs) in the questionable language\n
* @param correctness an optional pointer storing the correctness, or NULL\n
*
*  Ask google what language the snippet given as const char is written in \n
*  Optionally you can give a float pointer where the probability of the   \n
*  result being correct is stored in, (from 0.0 to 1.0), long snippets    \n
*  usually give better results. The result is returned as newly allocated \n
*  char * - free it.
*
* @return the language code ("en",de",..)
*/

char * Gly_gtrans_lookup(GlyQuery * s, const char * snippet, float * correctness);

/**
* @brief List all supported languages
*
* @param s The GlyQuery needed to determine the source/target language
*
* Queries google for a list of all supported languages \n
* Returned as a list of char pointers, free both the   \n
* single strings and the return value.                 \n
* If you only want to know all languages:            \n
* 'glyrc gtrans list' does the same.                  \n
*
* @return a list of strings, containg the language codes
*/

char ** Gly_gtrans_list(GlyQuery * s);

#endif
