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

#ifndef CORE_H
#define CORE_H

#include "types.h"
#include "apikeys.h"
#include "config.h"

/* libcurl */
#include <curl/curl.h>
#include <glib.h>

// Nifty defines
#define ABS(a)  (((a) < 0) ? -(a) : (a))
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

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

/* Group names */
#define GRPN_NONE "none"
#define GRPN_SAFE "safe"
#define GRPN_USFE "unsafe"
#define GRPN_SPCL "special"
#define GRPN_FAST "fast"
#define GRPN_SLOW "slow"
#define GRPN_ALL  "all"


// libglyr uses checksums to filter double items
// Also you can use those as easy comparasion method
// There is no valid reason to diasable this actually
#define CALC_MD5SUMS true

struct cb_object;

/**
* @brief The return type of Gly_get, a list of GlyMemCaches
*/
typedef struct GlyCacheList
{
    GlyMemCache ** list; /*!< A list of pointers to GlyMemCache instances, use Gly_clist_at to access those. */
    size_t size;	     /*!< Total number of items */
    int usersig;	     /*!< Do not use. */
} GlyCacheList;


/**
* @brief Structure holding information about built-in getters and providers
*
* Holding information about plugin-name, shortcut (key = "a" => "amazon"), a colored version of the name.
* You shouldn't bother with the rest
*
*/
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

    char gid;

} GlyPlugin;

/*------------------------------------------------------*/

// Internal representation of one metadataprovider
typedef struct MetaDataFetcher
{
   /* cover, lyrics, stuff */
   const char * name;
 
   /* will be replaced */
   GList * provider;

   /* what this thing delievers */
   enum GLYR_GET_TYPE type; 

   /* callbacks */
   bool (*validate)(GlyQuery *); 
   void (*init)(void);  
   void (*destroy)(void);
   GlyCacheList* (*finalize)(GlyCacheList*,GlyQuery*);

} MetaDataFetcher;

/*------------------------------------------------------*/

// Internal representation of one provider
typedef struct MetaDataSource {
      const char * name; /* Name of this provider            */
      char key; 	 /* A key that may be used in --from */
      
      GlyCacheList * (* parser) (struct cb_object *); /* called when parsing is needed                  */
      const char   * (* get_url)(GlyQuery *); 	      /* called when the url of this provider is needed */
      const char   * endmarker;                       /* Download stops if this mark is found           */

      enum GLYR_GET_TYPE type; /* For what fetcher this provider is working.. */

      int priority;  /* What priority this plugin has            */

      bool isUsed;   /* is used in searching? - set by .init     */
      bool free_url; /* URL is dyn. allocated - set this always! */

} MetaDataSource;

/*------------------------------------------------------*/

// Internal calback object, used for cover, lyrics and other
// This is only used inside the core and the plugins
// Other parts of the program shall not use this struct
// GlyMemCache is what you're searching
// It models the data that one plugin needs.
typedef struct cb_object
{
    // What callback to call
    GlyCacheList * (* parser_callback) (struct cb_object *);

    // What url to download before the callback is called
    char *url;

    // pointer to settings struct (artist,album,etc)
    GlyQuery * s;

    // What curl handle this is attached
    CURL *handle;

    GlyPlugin * plug;

    // internal cache attached to this url
    GlyMemCache *cache;

    // shall invoke() use a batch-like mode?
    // This usually only affects the output
    bool batch;

    const char * endmark;

    // This is only used for cover/photo
    // to fill the 'prov' field of memcache
    // This is a bit hackish, but well...
    // no valid reason to change it just because this ;)
    const char * provider_name;

} cb_object;

/*------------------------------------------------------*/

// Internal list of errors
// Use those with DL_error(ecode)
enum CORE_ERR
{
    ALL_OK,
    NO_BEGIN_TAG,
    NO_ENDIN_TAG,
    DOUBLE_ITEM,
    BAD_FORMAT,
    BLACKLISTED
};

/*------------------------------------------------------*/

// Check if a plugin needs to search more items
bool continue_search(int iter, GlyQuery * s);

// This needs to be called for each getter in the get_$getter() call. It sets up everything invoke() needs
GlyCacheList * register_and_execute(GlyQuery * settings, GlyCacheList * (* finalizer) (GlyCacheList *, GlyQuery *));
void plugin_init(cb_object *ref, const char *url, GlyCacheList * (callback)(cb_object*), GlyQuery * s, GlyPlugin * plug, const char * endmark, const char * prov_name, bool batch);

// download related methods
GlyCacheList * invoke(cb_object *oblist, long CNT, long parallel, long timeout, GlyQuery * s);
GlyMemCache * download_single(const char* url, GlyQuery * s, const char * end);

// cache related functions
GlyMemCache * DL_init(void);
GlyMemCache * DL_error(int eid);
GlyMemCache * DL_copy(GlyMemCache * src);
void DL_free(GlyMemCache *cache);

// Calculate a md5sum of the data
void update_md5sum(GlyMemCache * c);

// cacheList related methods
GlyCacheList * DL_new_lst(void);
void DL_free_lst(GlyCacheList * c);
void DL_add_to_list(GlyCacheList * l, GlyMemCache * c);
void DL_free_container(GlyCacheList * c);

// Copy a plugin_t * struct to a newly alloc'd bufer
GlyPlugin * copy_table(const GlyPlugin * o, size_t size);

// Verbosity fix
int glyr_message(int v, GlyQuery * s, FILE * stream, const char * fmt, ...);

// Control for bad items
int flag_lint(GlyCacheList * result, GlyQuery * s);
int flag_blacklisted_urls(GlyCacheList * result, const char ** URLblacklist, GlyQuery * s);
int flag_invalid_format(GlyCacheList * result, GlyQuery * s);

// Basic placeholder doing the job for review,tracklist..
GlyCacheList * generic_finalizer(GlyCacheList * result, GlyQuery * settings, int type);
GList * start_engine(GlyQuery * query, MetaDataFetcher * fetcher);

// just for fromoption...
const char * grp_id_to_name(int id);
#endif
