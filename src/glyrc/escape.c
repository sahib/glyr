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

#include <glib.h>
#include <string.h>
#include <stdio.h>

#include "escape.h"

static int     global_item_counter = 0;
static char *  global_write_path   = NULL;

#define EXPAND_FUNC(NAME)    \
static char * expand_##NAME( \
        GlyrQuery * q,       \
        GlyrMemCache * c     \
        )

EXPAND_FUNC(type) {
    return g_strdup(glyr_get_type_to_string(q->type));
}

EXPAND_FUNC(artist) {
    return g_strdup(q->artist);
}

EXPAND_FUNC(album) {
    return g_strdup(q->album);
}

EXPAND_FUNC(title) {
    return g_strdup(q->title);
}

EXPAND_FUNC(size) {
    return g_strdup_printf("%lu",c->size);
}

EXPAND_FUNC(format) {
    return c->is_image ? g_strdup(c->img_format) : g_strdup("txt");
}

EXPAND_FUNC(source) {
    return g_strdup(c->dsrc);
}

EXPAND_FUNC(prov) {
    return g_strdup(c->prov);
}

EXPAND_FUNC(cksum) {
    return glyr_md5sum_to_string(c->md5sum);
}

EXPAND_FUNC(rating) {
    return g_strdup_printf("%d",c->rating);
}

EXPAND_FUNC(duration) {
    return g_strdup_printf("%d",c->duration);
}

EXPAND_FUNC(number) {
    return g_strdup_printf("%d",global_item_counter);
}

EXPAND_FUNC(path) {
    return g_strdup(global_write_path);
}

////////////////////////

typedef char * (* ExpandFunction)(GlyrQuery *,GlyrMemCache *);

#define EXPAND(tname,escp) { \
    .name = #tname,          \
    .func = expand_##tname,  \
    .escape_slashes = escp   \
}                            \

struct escape_table {
    const char * name;
    ExpandFunction func;
    bool escape_slashes;
} _escapes[] = {
    EXPAND(type,false),
    EXPAND(artist,true),
    EXPAND(album,true),
    EXPAND(title,true),
    EXPAND(size,false),
    EXPAND(format,false),
    EXPAND(source,false),
    EXPAND(prov,false),
    EXPAND(cksum,false),
    EXPAND(rating,false),
    EXPAND(duration,false),
    EXPAND(number,false),
    EXPAND(path,false)
};

const size_t _escape_table_size = (sizeof(_escapes)/sizeof(struct escape_table));

////////////////////////

static void replace_char(char * string, gchar a, gchar b)
{
    if(string != NULL)
    {
        gsize str_len = strlen(string);
        for(size_t i = 0; i < str_len; i++)
        {
            if(string[i] == a)
            {
                string[i] = b;
            }
        }
    }
}

////////////////////////

static char * lookup_escape(const char * escape, size_t escape_len, GlyrQuery * q,  GlyrMemCache * c, bool * is_valid)
{
    for(size_t i = 0; i < _escape_table_size; ++i)
    {
        if(_escapes[i].name == NULL)
            continue;

        if(strncmp(escape,_escapes[i].name,escape_len) == 0)
        {
            *is_valid = true;
            char * result = _escapes[i].func(q,c);
            if(_escapes[i].escape_slashes)
            {
                replace_char(result,'/','|');
            }
            return result;
        }
    }
    *is_valid = false;
    return NULL;
}

////////////////////////

static size_t estimate_alloc_size(const char * str)
{
    int colons = 0, normal = 0;
    while(*str)
        if(*str++ == ':')
            colons++;
        else
            normal++;

    return (colons / 2 + 1) * 128 + normal + 1;
}

////////////////////////

char * escape_colon_expr(const char * path, GlyrQuery * q, GlyrMemCache * c)
{
    char * retv = NULL;
    if(path && q && c)
    {
        size_t maxbufsize = estimate_alloc_size(path);
        size_t off = 0;
        bool always_copy = false;
        retv = g_malloc0(maxbufsize+1);

        size_t path_len = strlen(path);
        for(size_t i = 0; i < path_len; ++i)
        {
            if(always_copy == false && path[i] == ':')
            {
                const char * escape_begin = &path[i+1];
                char * end_colon = strchr(escape_begin,':');
                if(end_colon != NULL)
                {
                    bool is_valid = false;
                    size_t escape_len = end_colon - escape_begin;
                    if(escape_len == 0) 
                        continue;

                    char * subsitution = lookup_escape(escape_begin,escape_len,q,c,&is_valid);
                    if(subsitution != NULL) 
                    {
                        size_t subs_len = strlen(subsitution);
                        memcpy(&retv[off],subsitution,subs_len);
                        off += subs_len;
                        g_free(subsitution);
                    }
                    else if(is_valid == false)
                    {
                        strncpy(retv+off,&path[i],escape_len + 2);
                        off += escape_len + 2;
                    }
                    i += escape_len + 1;
                }
                else
                {
                    always_copy = true;
                    i--;
                }
            }
            else
            {
                retv[off++] = path[i];
            }

            if(off >= maxbufsize) 
            {
                fprintf(stderr,"ERROR: Ouch! Buffersize too small. Hack: Add some :: to your string.\n");
                break;
            }
        }
    }
    if(q && q->verbosity >= 4) {
        printf(">>> %s\n",retv);
    }
    return retv;
}

////////////////////////

void increment_item_counter(void)
{
    ++global_item_counter;
}

////////////////////////

int get_item_counter(void)
{
    return global_item_counter;
}

////////////////////////

char * get_write_path(void)
{
    return global_write_path;
}

////////////////////////

void set_write_path(char * path)
{
    if(global_write_path != NULL)
        g_free(global_write_path);

    global_write_path = path;
}
