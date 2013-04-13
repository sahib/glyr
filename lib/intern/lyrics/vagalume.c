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

/* JSON Parsing */
#include "../../jsmn/jsmn.h"

#define VAGALUME_DOMAIN "http://www.vagalume.com.br"
#define VAGALUME_PATH "/api/search.php?art=${artist}&mus=${title}"

static const char * lyrics_vagalume_url (GlyrQuery * s)
{
    return VAGALUME_DOMAIN VAGALUME_PATH;
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
				if (len == 13 && g_ascii_strncasecmp(text_start, "song_notfound", len) == 0) {
					/* song not found! */
					break;
				} else {
					/* FIXME the only other "type" I've seen is "exact",
					 * don't know if there are any others. Maybe log the
					 * value if it is not "exact"? */
					song_found = true;
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
					cache->data = g_strndup(lyrics_start, len);
					cache->size = len;
					result_list = g_list_prepend(result_list, cache);
				}
			}
		}
	} else {
		/* not handled */
		fprintf(stderr, "Not handled\n");
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
