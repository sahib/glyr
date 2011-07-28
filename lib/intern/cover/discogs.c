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

#define RELEASE_ID  "<release id=\""
#define RELEASE_END "\" "
#define TITLE_BEGIN "<title>"
#define TITLE_ENDIN "</title>"
#define IMGURL_BEGIN "<image height=\""
#define IMGURL_ENDIN "\" type=\""
#define URL_BEGIN "uri=\""
#define URL_ENDIN "\" uri150="
#define IMG_TYPE "primary"

#define MAX_TRIES 5

#define API_KEY API_KEY_DISCOGS

const char * cover_discogs_url(GlyQuery * sets)
{
    if(sets->cover.max_size >= 300 || sets->cover.max_size == -1)
    {
        return "http://www.discogs.com/artist/%artist%?f=xml&api_key="API_KEY;
    }

    return NULL;
}

// The discogs.com parser is a little more complicated...
GlyCacheList * cover_discogs_parse(cb_object * capo)
{
    GlyCacheList * r_list=NULL;

    // Go through all release node
    int urlc = 0;
    char * release_node = capo->cache->data;
    while((release_node = strstr(release_node+1,RELEASE_ID)) != NULL && continue_search(urlc,capo->s))
    {
        // Find title in node
        char * title_begin = strstr(release_node,TITLE_BEGIN);
        if(!title_begin) continue;

        // Find end of value
        char * title_endin = strstr(release_node,TITLE_ENDIN);
        if(!title_endin) continue;

        // Go to beginning of value and get it
        char * title_value = copy_value(title_begin + strlen(TITLE_BEGIN),title_endin);
        if(title_value)
        {
            // Assure ptr not being reused
            title_begin = NULL;
            title_endin = NULL;

            // Compare with levenshtein
            if(levenshtein_strcmp(title_value,capo->s->album) <= capo->s->fuzzyness - 1)
            {
                // Get release ID
                char * release_end = strstr(release_node, RELEASE_END);
                if(release_end)
                {
                    // Copy ID from cache
                    char * release_ID = copy_value(release_node+strlen(RELEASE_ID),release_end);
                    if(release_ID)
                    {
                        release_end = NULL;

                        // Construct release_url
                        char *release_url = strdup_printf("http://www.discogs.com/release/%s?f=xml&api_key="API_KEY,release_ID);
                        if(release_url)
                        {
                            // Only download till artists tag.
                            GlyMemCache * tmp_cache = download_single(release_url,capo->s,"<artists>");
                            if(tmp_cache && tmp_cache->data && tmp_cache->size)
                            {
                                // Parsing the image url from here on
                                char * imgurl_begin = tmp_cache->data;
                                while((imgurl_begin = strstr(imgurl_begin+1,IMGURL_BEGIN)) != NULL)
                                {
                                    int img_type;

                                    char * is_primary = strstr(imgurl_begin,IMG_TYPE);
                                    if( ((is_primary - imgurl_begin) > 42) )
                                        continue;

                                    if(is_primary != NULL)
                                        img_type = TYPE_COVER_PRI;
                                    else
                                        img_type = TYPE_COVER_SEC;

                                    char * imgurl_endin = strstr(imgurl_begin,IMGURL_ENDIN);
                                    if(imgurl_endin)
                                    {
                                        char * size = copy_value(imgurl_begin+strlen(IMGURL_BEGIN),imgurl_endin);
                                        if(size)
                                        {
                                            int iS = atoi(size);
                                            if( (capo->s->cover.min_size == -1 && capo->s->cover.max_size == -1) ||
                                                    (capo->s->cover.min_size == -1 && capo->s->cover.max_size >= iS) ||
                                                    (capo->s->cover.min_size <= iS && capo->s->cover.max_size == -1) ||
                                                    (capo->s->cover.min_size <= iS && capo->s->cover.max_size >= iS)  )
                                            {
                                                char * uri_begin = strstr(imgurl_endin,URL_BEGIN);
                                                if(uri_begin)
                                                {
                                                    char * uri_endin = strstr(uri_begin,URL_ENDIN);
                                                    if(uri_endin)
                                                    {
                                                        char * url = copy_value(uri_begin+strlen(URL_BEGIN),uri_endin);
                                                        if(url)
                                                        {
                                                            GlyMemCache * result = DL_init();
                                                            if(result)
                                                            {
                                                                result->data = url;
                                                                result->size = uri_endin - (uri_begin + strlen(URL_BEGIN));
                                                                result->type = img_type;

                                                                if(!r_list) r_list = DL_new_lst();
                                                                DL_add_to_list(r_list,result);

                                                                urlc++;
                                                            }
                                                            else
                                                            {
                                                                free(url);
                                                                url=NULL;
                                                            }
                                                        }

                                                    }
                                                    uri_endin=NULL;
                                                }
                                                uri_begin=NULL;
                                            }
                                        }

                                        free(size);
                                        size=NULL;
                                    }
                                }
                                DL_free(tmp_cache);
                                tmp_cache=NULL;
                            }

                            free(release_url);
                            release_url=NULL;
                        }

                        free(release_ID);
                        release_ID=NULL;
                    }
                }
            }

            free(title_value);
            title_value = NULL;
        }
    }

    return r_list;
}

MetaDataSource cover_discogs_src = {
	.name      = "discogs",
	.key       = 'd',
	.parser    = cover_discogs_parse,
	.get_url   = cover_discogs_url,
	.type      = GET_COVERART,
	.endmarker = NULL,
	.free_url  = false
};
