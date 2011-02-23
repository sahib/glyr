#ifndef TYPES_H
#define TYPES_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <curl/curl.h>

#include "config.h"

//Nifty defines:
#define ABS(a)  (((a) < 0) ? -(a) : (a))

#define PRT_COLOR glyr_USE_COLOR
#define USE_COLOR
#ifdef  USE_COLOR
#define C_B "\x1b[34;01m" // blue
#define C_M "\x1b[35;01m" // magenta
#define C_C "\x1b[36;01m" // Cyan
#define C_R "\x1b[31;01m" // Red
#define C_G "\x1b[32;01m" // Green
#define C_Y "\x1b[33;01m" // Yellow
#define C_  "\x1b[0m"     // Reset
#else
#define C_B "" // blue
#define C_M "" // magenta
#define C_C "" // Cyan
#define C_R "" // Red
#define C_G "" // Green
#define C_Y "" // Yellow
#define C_  "" // Reset
#endif

// Change this if you really need more
#define WHATEVER_MAX_PLUGIN 16

#define DEFAULT_TIMEOUT  20L
#define DEFAULT_REDIRECTS 1L
#define DEFAULT_PARALLEL 4L
#define DEFAULT_CMINSIZE 125
#define DEFAULT_CMAXSIZE -1
#define DEFAULT_VERBOSITY 2
#define DEFAULT_NUMBER 1
#define DEFAULT_PLUGMAX 7
#define DEFAULT_LANG GLYRL_UK
#define DEFAULT_DOWNLOAD true

enum GLYR_LANG
{
    GLYRL_US,
    GLYRL_CA,
    GLYRL_UK,
    GLYRL_FR,
    GLYRL_DE,
    GLYRL_JP,
};

// seperator for providerlist
#define FROM_ARGUMENT_DELIM ";"

#define PTR_SPACE 10

// Prototype cb_object struct
struct cb_object;

// Note:
// you will notice that some structs share the same data,
// the more specialized the struct gets the more obsucre members it have
// glyr_settings_t -> cb_object e.g.

// Internal buffer model
typedef struct memCache_t
{
    char  *data;   // data buffer
    size_t size;   // Size of data
    char  *dsrc;   // Source of data
} memCache_t;

// list of memCache_ts
typedef struct cache_list
{
    memCache_t ** list;
    size_t size;
} cache_list;

typedef struct glyr_settings_t
{
    // get
    int type;

    const char * info[PTR_SPACE];

    // of
    const char * artist;
    const char * album;
    const char * title;

    // number
    int number;
    int plugmax;

    // cover
    struct
    {
        int min_size;
        int max_size;
    } cover;

    // from (as void because I didnt manage to prototype plugin_t... *?*)
    void * providers;

    // invoke() control
    long parallel;
    long timeout;
    long redirects;

    // verbosity
    int verbosity;
    // use colored output?
    bool color_output;

    // return only urls without downloading?
    // this converts glyr to a sort of search engines
    bool download;

    // language settings (for amazon / google / last.fm)
    int lang;

    struct callback {
        void (* download)(memCache_t * dl, struct glyr_settings_t * s);
        void  * user_pointer;
    } callback;

} glyr_settings_t;

// The struct that controls the beahaviour of glyr
// It is passed as reference to the cover and lyric downloader
// Also the descriptive argumentstring is internally converted to a
// glyr_settings_t first

typedef struct plugin_t
{
    const char * name;  // Full name
    const char * key;   // A one-letter ID
    const char * color; // Colored name
    int use;            // Use this source?

    struct
    {
        // Passed to the corresponding cb_object and is called...perhaps
        cache_list * (* parser_callback) (struct cb_object *);
        const char * (* url_callback)    (glyr_settings_t  *);
        bool free_url; // pass result of url_callback to free()?
    } plug;

} plugin_t;

enum GLYR_GET_TYPES
{
    GET_COVER,
    GET_LYRIC,
    GET_PHOTO,
    GET_AINFO,
    GET_BOOKS,
    GET_UNSURE
};

#endif
