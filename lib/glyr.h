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
#ifndef GLYR_H
#define GLYR_H

// va_list for Gly_setopt()
#include <stdarg.h>

// all structs used by glyr are here
#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*callback_t) (void *user_data, const char *other_data);

    // the actual main of glyr
    GlyCacheList * Gly_get(GlyQuery * settings, int * error);

    // Initialize settings struct to sane defaults
    void Gly_init_query(GlyQuery * glyrs);

    // free all memory that may been still allocated in struct
    void Gly_destroy_query(GlyQuery * sets);

    // free results
    void Gly_free_list(GlyCacheList * lst);
    void Gly_free_cache(GlyMemCache * c);
    void Gly_push_to_list(GlyCacheList * l, GlyMemCache * c);
    GlyMemCache * Gly_clist_at(GlyCacheList * clist, int iter);
    GlyMemCache * Gly_new_cache(void);

    /* Gly_opt_* methods */
    int GlyOpt_dlcallback(GlyQuery * settings, int (*dl_cb)(GlyMemCache *, GlyQuery *), void * userp);
    int GlyOpt_type(GlyQuery * s, int type);
    int GlyOpt_artist(GlyQuery * s, char * artist);
    int GlyOpt_album(GlyQuery * s,  char * album);
    int GlyOpt_title(GlyQuery * s,  char * title);
    int GlyOpt_cmaxsize(GlyQuery * s, int size);
    int GlyOpt_cminsize(GlyQuery * s, int size);
    int GlyOpt_parallel(GlyQuery * s, unsigned long val);
    int GlyOpt_timeout(GlyQuery * s, unsigned long val);
    int GlyOpt_redirects(GlyQuery * s, unsigned long val);
    int GlyOpt_lang(GlyQuery * s, char * langcode);
    int GlyOpt_number(GlyQuery * s, unsigned int num);
    int GlyOpt_verbosity(GlyQuery * s, unsigned int level);
    int GlyOpt_infoat(GlyQuery * s, int at, const char * value);
    int GlyOpt_from(GlyQuery * s, const char * from);
    int GlyOpt_color(GlyQuery * s, bool iLikeColorInMyLife);
    int GlyOpt_plugmax(GlyQuery * s, int plugmax);
    int GlyOpt_download(GlyQuery * s, bool download);
    int GlyOpt_groupedDL(GlyQuery * s, bool groupedDL);
    int GlyOpt_formats(GlyQuery * s, const char * formats);
    int GlyOpt_fuzzyness(GlyQuery * s, int fuzz);
    int GlyOpt_call_direct_use(GlyQuery * s, bool use);
    int GlyOpt_call_direct_provider(GlyQuery * s, const char * provider);
    int GlyOpt_call_direct_url(GlyQuery * s, const char * URL);

    // return library version
    const char * Gly_version(void);

    // print method that works with verbosity
    int glyr_message(int v, GlyQuery * s, FILE * stream, const char * fmt, ...);

    // get information about available plugins
    GlyPlugin * Gly_get_provider_by_id(int ID);

    // Download a URL and sae it in Memcache
    GlyMemCache * Gly_download(const char * url, GlyQuery * s);

    // write binary file, this is for use in language bindings mainly, which partly can't easily write them themself
    int Gly_write_binary_file(const char * path, GlyMemCache * data, const char * save_dir, const char * type, GlyQuery *s);

    // Returns the actual name of the group pointed by ID
    const char * Gly_groupname_by_id(int ID);

#ifdef _cplusplus
}
#endif

#endif
