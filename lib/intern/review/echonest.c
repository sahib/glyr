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
#include "../../core.h"
#include "../../stringlib.h"
#include "../../apikeys.h"

/* JSON Parsing */
#include "../../jsmn/jsmn.h"

static const gchar * review_echonest_url (GlyrQuery * s)
{
    return "http://developer.echonest.com/api/v4/artist/reviews?api_key="API_KEY_ECHONEST"&results=${number}&format=json&name=${artist}";
}

/////////////////////////////////

static int strip_escapes(char * text, int len) 
{
    int offset = 0;

    for (int i = 0; i < len; ++i) {
        if (text[i] == '\\') {
            ++i;
            if (text[i] == 'n') {
                text[offset++] = '\n';
                continue;
            }
        }
        text[offset++] = text[i];
    }

    return len - offset;
}

/////////////////////////////////

static GList *add_result(GList *results, cb_object *capo, char *url, char *summary, char *release) {
    if (levenshtein_strnormcmp(capo->s, capo->s->album, release) <= capo->s->fuzzyness) {
        if (strlen(summary) >= 125) {
            GlyrMemCache * result = DL_init();
            result->data = g_strdup_printf("%s\n \n \nRead more on: %s\n", summary, url);
            result->size = strlen(result->data);
            result->size -= strip_escapes(result->data, result->size);
            return g_list_prepend(results, result);
        }
    }
    return results;
}

/////////////////////////////////

static bool json_is_key(jsmntok_t tokens[], int current, char *json, char *key) 
{
    jsmntok_t * token = &tokens[current];
    return (token->type == JSMN_STRING &&
            g_ascii_strncasecmp(
                json + token->start,
                key,
                token->end - token->start) == 0
           );
}

/////////////////////////////////

static char * json_get_next_value(jsmntok_t tokens[], int current, char *json) 
{
    jsmntok_t *next_token = &tokens[current + 1];
    if (next_token != NULL && next_token->type == JSMN_STRING) {
        return g_strndup(json + next_token->start, next_token->end - next_token->start);
    } else {
        return NULL;
    }
}

/////////////////////////////////

#define JSON_GET_VALUE_IF_KEY(name, key)                  \
    if (json_is_key(tokens, i, json, key)) {              \
        g_free(name);                                     \
        name = json_get_next_value(tokens, i++, json);    \
    }                                                     \

static GList * review_echonest_parse (cb_object * capo)
{
    char * json = capo->cache->data;
    const int num_tokens = 512;
    

    GList * results = NULL;

    /* jasmin stuff */
    jsmn_parser parser;
    jsmntok_t tokens[num_tokens];
    jsmnerr_t error;

    /* Init the parser */
    jsmn_init(&parser);

    /* make sure it terminates */
    memset(tokens, 0, num_tokens * sizeof(jsmntok_t));

    /* Parse the json text */
    error = jsmn_parse(&parser, capo->cache->data, tokens, num_tokens);

    /* Some manual parsing required */
    char *curr_bracket = NULL;

    /* Save partial results here */
    char *url = NULL, *summary = NULL, *release = NULL;

    if (error == JSMN_SUCCESS) {
        for (int i = 0; i < num_tokens; ++i) {
            jsmntok_t * token = &tokens[i];

            /* End of tokens? */
            if(token->start <= 0 && token->end <= 0) {
                break;
            }

            JSON_GET_VALUE_IF_KEY(url, "url");
            JSON_GET_VALUE_IF_KEY(summary, "summary");
            JSON_GET_VALUE_IF_KEY(release, "release");

            char * next_bracket = strchr(json + token->start, '}');
            if (next_bracket > curr_bracket) {
                if (url && summary && release) {
                    results = add_result(results, capo, url, summary, release);
                }
                curr_bracket = next_bracket;
            }

        }

        if (url && summary && release) {
            results = add_result(results, capo, url, summary, release);
        }

        g_free(url);
        g_free(summary);
        g_free(release);
    } else {
        /* No intelligent error handling yet. */
    }

    return results;
}

/////////////////////////////////

MetaDataSource review_echonest_src =
{
    .name      = "echonest",
    .key       = 'e',
    .free_url  = false,
    .type      = GLYR_GET_ALBUM_REVIEW,
    .parser    = review_echonest_parse,
    .get_url   = review_echonest_url,
    .quality   = 70,
    .speed     = 85,
    .endmarker = NULL,
    .lang_aware = false 
};
