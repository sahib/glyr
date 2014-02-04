/***********************************************************
 * This file is part of glyr
 * + a commnadline tool and library to download various sort of music related metadata.
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

#include "../../core.h"
#include "../../stringlib.h"

/* JSON Parsing */
#include "../../jsmn/jsmn.h"

#define VAGALUME_DOMAIN "http://www.vagalume.com.br"
#define VAGALUME_PATH "/api/search.php?art=${artist}&mus=${title}"

/**
 * Vagalume Provider written by: Felipe Bessa Coelho
 * 
 * https://github.com/fcoelho
 *
 * Many thanks.
 */

static const char * lyrics_vagalume_url (GlyrQuery * s)
{
    return VAGALUME_DOMAIN VAGALUME_PATH;
}

/* This was taken from here:
 * https://git.gnome.org/browse/json-glib/tree/json-glib/json-scanner.c#n558 */
static gunichar get_unichar(const char *src)
{
    gunichar uchar;
    gchar ch;
    gint i;

    uchar = 0;
    for (i = 0; i < 4; ++i) {
        ch = src[i];
        if (g_ascii_isxdigit(ch))
            uchar += ((gunichar) g_ascii_xdigit_value(ch) << ((3 - i) * 4));
        else
            break;
    }

    return uchar;
}

static GString * vagalume_escape_text(const char * src, int len)
{
    GString * output = g_string_sized_new(len);

    for (int i = 0; i < len; ++i) {
        if (src[i] == '\\') {
            ++i;
            if (src[i] == 'n') {
                output = g_string_append_c(output, '\n');
                continue;
            } else if (src[i] == 'u') {
                ++i;
                /* this was taken from here:
                 * https://git.gnome.org/browse/json-glib/tree/json-glib/json-scanner.c#n1116 */
                gunichar ucs = get_unichar(src + i);
                /* 4 characters read, the XXXX in \uXXXX, but we leave the last
                 * increment to the for loop */
                i += 3;
                if (g_unichar_type(ucs) == G_UNICODE_SURROGATE) {
                    /* read next surrogate, but first update our source string
                     * pointer */
                    ++i;
                    if ('\\' == src[i++] && 'u' == src[i++]) {
                        gunichar ucs_lo = get_unichar(src + i);
                        /* 6 characters read from \uXXXX: '\\', 'u' and XXXX,
                         * but again, leave the last one to be updated by the
                         * for loop*/
                        i += 5;

                        ucs = (((ucs & 0x3ff) << 10) | (ucs_lo & 0x3ff)) + 0x10000;
                    }
                }

                output = g_string_append_unichar(output, ucs);
            }
        } else {
            output = g_string_append_c(output, src[i]);
        }
    }

    return output;
}

static gboolean found_song (const char *json, jsmntok_t *tokens, int *cur_token, int *tokens_left)
{
    gboolean found_key = false, song_found = false;

    /* check "type" key to see if song was found */
    for (*cur_token = 0, *tokens_left = 1; *tokens_left > 0; ++(*cur_token), --(*tokens_left)) {
        jsmntok_t * token = &tokens[*cur_token];

        if (token->type == JSMN_ARRAY || token->type == JSMN_OBJECT)
            *tokens_left += token->size;

        if (token->type == JSMN_STRING) {
            size_t len = token->end - token->start;
            const char *text_start = json + token->start;

            if (len == 4 && g_ascii_strncasecmp(text_start, "type", len) == 0) {
                found_key = true;
                continue;
            }
            if (found_key) {
                if (len == 5 && g_ascii_strncasecmp(text_start, "exact", len) == 0) {
                    song_found = true;
                    break;
                } else {
                    /* other types are notfound, song_notfound and aprox
                     * aprox results can be crazy, let's ignore them */
                    break;
                }
            }
        }
    }
    
    /* we return only here to make sure tokens_left is updated at the end of
     * the for loop (specially if we break out of it) */
    return song_found;
}

static jsmntok_t * get_text_token(const char *json, jsmntok_t *tokens, int *cur_token, int *tokens_left)
{
    gboolean found_key = false;
    jsmntok_t * text_token = NULL;

    /* find the first occurrence of "text" */
    for (/* empty */; *tokens_left > 0; ++(*cur_token), --(*tokens_left)) {
        jsmntok_t * token = &tokens[*cur_token];

        if (token->type == JSMN_ARRAY || token->type == JSMN_OBJECT)
            *tokens_left += token->size;

        if (token->type == JSMN_STRING) {
            size_t len = token->end - token->start;
            const char *text_start = json + token->start;

            if (len == 4 && g_ascii_strncasecmp(text_start, "text", len) == 0) {
                found_key = true;
                continue;
            }

            if (found_key) {
                /* this is it */
                text_token = token;
                break;
            }
        }
    }

    return text_token;
}

static GList * lyrics_vagalume_parse (cb_object * capo)
{
    GList * result_list = NULL;

    /* jsmn variables */
    jsmn_parser parser;
    jsmntok_t tokens[512];
    jsmnerr_t parse_result;

    /* used to track where in the json response we are */
    int cur_token, tokens_left;
    const char *json = capo->cache->data;

    /* Make sure tokens are initialized */
    memset(&tokens, 0, sizeof(jsmntok_t) * 512);

    jsmn_init(&parser);

    parse_result = jsmn_parse(&parser, json, tokens, 512);

    if (parse_result == JSMN_SUCCESS)
    {
        gboolean song_found = found_song(json, tokens, &cur_token, &tokens_left);
        if (song_found) {
            jsmntok_t * text_token = get_text_token(json, tokens, &cur_token, &tokens_left);
            if (text_token) {
                const char * lyrics_start = json + text_token->start;
                size_t len = text_token->end - text_token->start;

                GlyrMemCache * cache = DL_init();
                if (cache != NULL) {
                    GString * data = vagalume_escape_text(lyrics_start, len);

                    cache->data = data->str;
                    cache->size = data->len;
                    result_list = g_list_prepend(result_list, cache);

                    /* Free string wrapper */
                    g_string_free(data, FALSE);
                }
            }
        }
    } else {
        /* not handled */
    }

    return result_list;
}

/////////////////////////////////

MetaDataSource lyrics_vagalume_src =
{
    .name = "vagalume",
    .key  = 'g',
    .parser    = lyrics_vagalume_parse,
    .get_url   = lyrics_vagalume_url,
    .type      = GLYR_GET_LYRICS,
    .quality   = 90,
    .speed     = 90,
    .endmarker = NULL,
    .free_url  = false
};
