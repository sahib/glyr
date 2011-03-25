/***********************************************************
* This file is part of glyr
* + a commnadline tool and library to download various sort of musicrelated metadata.
* + Copyright (C) [2011]  [Christopher Pahl]
* + Hosted at: https://github.com/sahib/glyr
*
* glyr is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* glyr is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with glyr. If not, see <http://www.gnu.org/licenses/>.
**************************************************************/
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
#define BETWEEN(val,low,up) (val >= low && val <= up)

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

#define GOOGLE_COLOR C_B"g"C_R"o"C_Y"o"C_B"g"C_G"l"C_R"e"

// Change this if you really need more
#define WHATEVER_MAX_PLUGIN 32

#define DEFAULT_TIMEOUT  20L
#define DEFAULT_REDIRECTS 1L
#define DEFAULT_PARALLEL 4L
#define DEFAULT_CMINSIZE 125
#define DEFAULT_CMAXSIZE -1
#define DEFAULT_VERBOSITY 0
#define DEFAULT_NUMBER 1
#define DEFAULT_PLUGMAX -1
#define DEFAULT_LANG "en"
#define DEFAULT_DOWNLOAD true
#define DEFAULT_GROUPEDL true
#define DEFAULT_FROM_ARGUMENT_DELIM ";"
#define DEFAULT_DUPLCHECK true
#define DEFAULT_FUZZYNESS 4
#define DEFAULT_FORMATS "jpg;jpeg;png"
#define DEFAULT_CALL_DIRECT_USE false
#define DEFAULT_CALL_DIRECT_PROVIDER NULL

#define PTR_SPACE 10

#define API_KEY_DISCOGS "adff651383"
#define API_KEY_AMAZON  "AKIAJ6NEA642OU3FM24Q"
#define API_KEY_LASTFM  "7199021d9c8fbae507bf77d0a88533d7"
#define API_KEY_FLICKR  "b5af0c3230fb478d53b20835223d57a4"

/* --------------------------- */
/* --------- GROUPS ---------- */
/* --------------------------- */

/* Group IDs */
enum GLYR_GROUPS
{
    GRP_NONE = 0 << 0, /* None    */
    GRP_SAFE = 1 << 0, /* Safe    */
    GRP_USFE = 1 << 1, /* Unsafe  */
    GRP_SPCL = 1 << 2, /* Special */
    GRP_FAST = 1 << 3, /* Fast    */
    GRP_SLOW = 1 << 4, /* Slow    */
    GRP_ALL  = 1 << 5  /* All!    */
};

enum GLYR_ERROR
{
    GLYRE_OK,           // everything is fine
    GLYRE_BAD_OPTION,   // you passed a bad option to Gly_setopt()
    GLYRE_BAD_VALUE,    // Invalid value in va_list
    GLYRE_EMPTY_STRUCT, // you passed an empty struct to Gly_setopt()
    GLYRE_NO_PROVIDER,  // setttings->provider == NULL
    GLYRE_UNKNOWN_GET,  // settings->type is not valid
    GLYRE_STOP_BY_CB    // Callback returned stop signal.
};

/* Group names */
#define GRPN_NONE "none"
#define GRPN_SAFE "safe"
#define GRPN_USFE "unsafe"
#define GRPN_SPCL "special"
#define GRPN_FAST "fast"
#define GRPN_SLOW "slow"
#define GRPN_ALL  "all"

// Prototype cb_object struct
struct cb_object;

// Note:
// you will notice that some structs share the same data,
// the more specialized the struct gets the more obsucre members it have
// GlyQuery -> cb_object e.g.

// Internal buffer model
typedef struct GlyMemCache
{
    char  *data;    // data buffer
    size_t size;    // Size of data
    char  *dsrc;    // Source of data
    int   type;     // type of metadata
    int   error;    // error code - internal use only
    int   duration; // Duration of a song. Only for tracklist getter.
} GlyMemCache;

// list of GlyMemCaches
typedef struct GlyCacheList
{
    GlyMemCache ** list;
    size_t size;
    int usersig;
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

    // Download group for group,
    // or all in parallel? (faster, but less accurate)
    bool groupedDL;

    // Check for bad data?
    bool duplcheck;

    // language settings (for amazon / google / last.fm)
    const char * lang;

    // count of dl'd items, starting from 0
    int itemctr;

    // Treshold for Levenshtein
    size_t fuzzyness;

    // allowed formats for images
    const char * formats;

    struct callback
    {
        int  (* download)(GlyMemCache * dl, struct GlyQuery * s);
        void  * user_pointer;
    } callback;

    struct call_direct
    {
        bool use;
        const char * provider;
        const char * url;
    } call_direct;

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
        const char *  endmarker; // Stop download if containing this string
        bool free_url; // pass result of url_callback to free()?
    } plug;

    unsigned char gid;

} GlyPlugin;

enum GLYR_GET_TYPE
{
    GET_COVER,
    GET_LYRIC,
    GET_PHOTO,
    GET_AINFO,
    GET_SIMILIAR,
    GET_REVIEW,
    GET_TRACKLIST,
    GET_TAGS,
    GET_ALBUMLIST,
    GET_UNSURE
};

// This is not a duplicate of GLYR_GET_TYPE
// (more to follow)
enum GLYR_DATA_TYPE
{
    TYPE_NOIDEA,
    TYPE_LYRICS,
    TYPE_REVIEW,
    TYPE_PHOTOS,
    TYPE_COVER,
    TYPE_COVER_PRI,
    TYPE_COVER_SEC,
    TYPE_AINFO,
    TYPE_SIMILIAR,
    TYPE_ALBUMLIST,
    TYPE_TAGS,
    TYPE_TAG_ARTIST,
    TYPE_TAG_ALBUM,
    TYPE_TAG_TITLE,
    TYPE_TRACK
};

#endif
