#ifndef TYPES_H
#define TYPES_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <curl/curl.h>

// cmake -Dglyr_USE_COLOR
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
#define WHATEVER_MAX_PLUGIN 32

#define DEFAULT_TIMEOUT  20L
#define DEFAULT_REDIRECTS 1L
#define DEFAULT_PARALLEL 4L
#define DEFAULT_CMINSIZE 125
#define DEFAULT_CMAXSIZE -1
#define DEFAULT_VERBOSITY 2
#define DEFAULT_NUMBER 1
#define DEFAULT_PLUGMAX 10 
#define DEFAULT_LANG "en"
#define DEFAULT_DOWNLOAD true
#define DEFAULT_FROM_ARGUMENT_DELIM ";"

#define PTR_SPACE 10

// Prototype cb_object struct
struct cb_object;

// Note:
// you will notice that some structs share the same data,
// the more specialized the struct gets the more obsucre members it have
// GlyQuery -> cb_object e.g.

// Internal buffer model
typedef struct GlyMemCache
{
    char  *data;   // data buffer
    size_t size;   // Size of data
    char  *dsrc;   // Source of data
    int   error;   // error code - internal use only
} GlyMemCache;

// list of GlyMemCaches
typedef struct GlyCacheList
{
    GlyMemCache ** list;
    size_t size;
} GlyCacheList;

typedef struct GlyQuery
{
    // get
    int type;

    const char * info[PTR_SPACE];

    // of
    char * artist;
    char * album;
    char * title;

    // number
    int number;
    int plugmax;

    // cover
    struct
    {
        int min_size;
        int max_size;
    } cover;

    // from (as void because I didnt manage to prototype GlyPlugin... *?*)
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
    char * lang;

    // count of dl'd items, starting from 0
    int itemctr;

    struct callback {
        void (* download)(GlyMemCache * dl, struct GlyQuery * s);
        void  * user_pointer;
    } callback;

} GlyQuery;

// The struct that controls the beahaviour of glyr
// It is passed as reference to the cover and lyric downloader
// Also the descriptive argumentstring is internally converted to a
// GlyQuery first

typedef struct GlyPlugin
{
    const char * name;  // Full name
    const char * key;   // A one-letter ID
    const char * color; // Colored name
    int use;            // Use this source?

    struct
    {
        // Passed to the corresponding cb_object and is called...perhaps
        GlyCacheList * (* parser_callback) (struct cb_object *);
        const char *   (* url_callback)    (GlyQuery  *);
        bool free_url; // pass result of url_callback to free()?
    } plug;

} GlyPlugin;

enum GLYR_GET_TYPES
{
    GET_COVER,
    GET_LYRIC,
    GET_PHOTO,
    GET_AINFO,
    GET_SIMILIAR,
    GET_REVIEW,
    GET_BOOKS,
    GET_UNSURE
};

#endif
