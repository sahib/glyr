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

    // Storage of the --of argument
    const char ** info;

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

// just for fromoption...
const char * grp_id_to_name(int id);
#endif
