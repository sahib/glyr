#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>

#include "discogs.h"

#include "../core.h"
#include "../core.h"
#include "../stringop.h"

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

#define LV_DISTANCE 3

#define API_KEY "adff651383"

const char * cover_discogs_url(glyr_settings_t * sets)
{
    if(sets->cover.max_size >= 300 || sets->cover.max_size == -1)
        return "http://www.discogs.com/artist/%artist%?f=xml&api_key="API_KEY;

    return NULL;
}


// The discogs.com parser is a little more complicated...
memCache_t * cover_discogs_parse(cb_object * capo)
{
    memCache_t * result=NULL;

    // Go through all release node
    int ctr = 0;
    char * release_node = capo->cache->data;
    while((release_node = strstr(release_node+1,RELEASE_ID)) != NULL && !result && MAX_TRIES >= ctr++)
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
            if(levenshtein_strcmp(title_value,capo->s->album) <= LV_DISTANCE)
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
                            memCache_t * tmp_cache = download_single(release_url,capo->s);
                            if(tmp_cache && tmp_cache->data && tmp_cache->size)
                            {
                                // Parsing the image url from here on
                                char * imgurl_begin = tmp_cache->data;
                                while((imgurl_begin = strstr(imgurl_begin+1,IMGURL_BEGIN)) != NULL && !result)
                                {
                                    // only use prmary images
                                    char * is_primary = strstr(imgurl_begin,IMG_TYPE);
                                    if(!is_primary || ((is_primary - imgurl_begin) > 42) )
                                        continue;

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
                                                            result = DL_init();
                                                            if(result)
                                                            {
                                                                result->data = url;
                                                                result->size = uri_endin - (uri_begin + strlen(URL_BEGIN));
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

    return result;
}
