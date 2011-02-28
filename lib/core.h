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

    // Custom pointer used to pass
    // unspecified data to finalize()
    void * custom;

    // pointer to settings struct (artist,album,etc)
    GlyQuery * s;

    //Plugin name and the colored version - only important for verbose output
    const char * name;
    const char * color;

    // What curl handle this is attached
    CURL *handle;

    // internal cache attached to this url
    GlyMemCache *cache;

} cb_object;

enum CORE_ERR
{
    ALL_OK,
    NO_BEGIN_TAG,
    NO_ENDIN_TAG,
    DOUBLE_ITEM,
    BLACKLISTED
};

bool continue_search(int iter, GlyQuery * s);

void plugin_init(cb_object *ref, const char *url, GlyCacheList * (callback)(cb_object*), GlyQuery * s, const char *name, const char * color, void * custom);
void glyr_init_settings(GlyQuery * glyrs);

GlyCacheList * register_and_execute(GlyQuery * settings, GlyCacheList * (* finalizer) (GlyCacheList *, GlyQuery *));

GlyCacheList * invoke(cb_object *oblist, long CNT, long parallel, long timeout, GlyQuery * s);
GlyMemCache * download_single(const char* url, GlyQuery * s);

GlyMemCache * DL_init(void);
GlyMemCache * DL_error(int eid);
GlyMemCache * DL_copy(GlyMemCache * src);

void DL_free(GlyMemCache *cache);

GlyCacheList * DL_new_lst(void);
void DL_free_lst(GlyCacheList * c);
void DL_add_to_list(GlyCacheList * l, GlyMemCache * c);
void DL_free_container(GlyCacheList * c);

GlyPlugin * copy_table(const GlyPlugin * o, size_t size);

int glyr_message(int v, GlyQuery * s, FILE * stream, const char * fmt, ...);

int flag_double_urls(GlyCacheList * result, GlyQuery * s);
int flag_blacklisted_urls(GlyCacheList * result, const char ** URLblacklist, GlyQuery * s);

#endif
