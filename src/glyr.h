#ifndef GLYR_H
#define GLYR_H

// glyr uses bool values
#include <stdbool.h>

// va_list for glyr_setopt()
#include <stdarg.h>

// all structs used by glyr are here
#include "types.h"

enum GLYR_AMAZON 
{
	GLYR_AMAZON_US,
	GLYR_AMAZON_CA,
	GLYR_AMAZON_UK,
	GLYR_AMAZON_FR,
	GLYR_AMAZON_DE,
	GLYR_AMAZON_JP
};

enum GLYR_OPTIONS
{
	// absolute values
	GLYR_OPT_TYPE,
	GLYR_OPT_ARTIST,
	GLYR_OPT_ALBUM,
	GLYR_OPT_TITLE,
	GLYR_OPT_CMINSIZE,
	GLYR_OPT_CMAXSIZE,
	GLYR_OPT_UPDATE,
	GLYR_OPT_PARALLEL,
	GLYR_OPT_AMAZON_ID,
	GLYR_OPT_NPHOTO,
	GLYR_OPT_PATH,
	GLYR_OPT_FROM,
	GLYR_OPT_OF
};
enum GLYR_ERROR
{
	GLYR_OK,           // everything is fine
	GLYR_BAD_OPTION,   // you passed a bad option to glyr_setopt()
	GLYR_EMPTY_STRUCT  // you passed an empty struct to glyr_setopt()
};

// the actual main of glyr
const char * glyr_get(glyr_settings_t * settings);

// Initialize settings struct to sane defaults
void glyr_init_settings(glyr_settings_t * glyrs);

// free all memory that may been still allocated in struct
void glyr_destroy_settings(glyr_settings_t * sets);

// set a specififc option
int glyr_setopt(glyr_settings_t * s, int option, ...);

#endif
