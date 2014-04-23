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
#include "../../apikeys.h"

/* JSON Parsing */
#include "../../jsmn/jsmn.h"

static const gchar * ainfo_echonest_url (GlyrQuery * s)
{
    return "http://developer.echonest.com/api/v4/artist/biographies?api_key="API_KEY_ECHONEST"&results=${number}&format=json&name=${artist}";
}

/////////////////////////////////

static char * echonest_strip_escapes(char * src, char * dest, int len) 
{
    int offset = 0;

    for (int i = 0; i < len; ++i) {
        if (src[i] == '\\') {
            ++i;
            if (src[i] == 'n') {
                dest[offset++] = '\n';
                continue;
            }
        }
        dest[offset++] = src[i];
    }

    return dest;
}

/////////////////////////////////

static bool echonest_check_if_text_is_valid(char * text, int len) 
{
    bool rc = false;
    if (len >= 125) {
        return true;
    }

    return rc;
}

/////////////////////////////////

static GList * ainfo_echonest_parse (cb_object * capo)
{
    char * json = capo->cache->data;
    bool is_text = false;
    const int num_tokens = 512;
    
    GList * results = NULL;

    /* jasmin stuff */
    jsmn_parser parser;
    jsmntok_t tokens[num_tokens];
    jsmnerr_t error;

    /* make sure it terminates */
    memset(tokens, 0, num_tokens * sizeof(jsmntok_t));

    /* Init the parser */
    jsmn_init(&parser);

    /* Parse the json text */
    error = jsmn_parse(&parser, capo->cache->data, tokens, num_tokens);

    if (error == JSMN_SUCCESS) {
        for (int i = 0; i < num_tokens; ++i) {
            jsmntok_t * tok = &tokens[i];
            size_t len = tok->end - tok->start;
            char * text_off = json + tok->start;
            
            /* End of tokens? */
            if(tok->start == 0 && tok->end == 0) {
                break;
            }

            /* Check for the "text" field. */
            if (tok->type == JSMN_STRING) {
                if (len == 4 && g_ascii_strncasecmp(text_off, "text", len) == 0) {
                    is_text = true;
                    continue;
                }
            }

            /* Interesting part! */
            if (is_text == true && tok->type == JSMN_STRING) {
                if (echonest_check_if_text_is_valid(text_off, len)) {
                    GlyrMemCache * cache = DL_init();
                    if (cache != NULL) {
                        cache->data = g_strndup(text_off, len);
                        cache->data = echonest_strip_escapes(cache->data, cache->data, len);
                        cache->size = len;
                        results = g_list_prepend(results, cache);
                    }
                }
                is_text = false;
                continue;
            }
        }
    } else {
        /* No intelligent error handling yet. */
    }

    return results;
}

/////////////////////////////////

MetaDataSource ainfo_echonest_src =
{
    .name      = "echonest",
    .key       = 'e',
    .free_url  = false,
    .type      = GLYR_GET_ARTIST_BIO,
    .parser    = ainfo_echonest_parse,
    .get_url   = ainfo_echonest_url,
    .quality   = 95,
    .speed     = 85,
    .endmarker = NULL,
    .lang_aware = false 
};
