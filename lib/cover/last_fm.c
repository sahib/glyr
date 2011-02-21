#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "last_fm.h"

#include "../cover.h"
#include "../stringop.h"
#include "../core.h"
#include "../core.h"

#define API_KEY "7199021d9c8fbae507bf77d0a88533d7"

const char * cover_lastfm_url(glyr_settings_t * sets)
{
    if(sets->cover.min_size <= 350 || sets->cover.min_size == -1)
    {
        return "http://ws.audioscrobbler.com/2.0/?method=album.search&album=%artist%+%album%&api_key="API_KEY;
    }
    return NULL;
}

cache_list * cover_lastfm_parse(cb_object *capo)
{
    // Handle size requirements (Default to large)
    char *tag_ssize = NULL ;
    char *tag_esize = "</image>";

    // find desired size
    if( size_is_okay(300,capo->s->cover.min_size,capo->s->cover.max_size) )
        tag_ssize = "<image size=\"extralarge\">";
    else if( size_is_okay(125,capo->s->cover.min_size,capo->s->cover.max_size) )
        tag_ssize = "<image size=\"large\">";
    else if( size_is_okay(64, capo->s->cover.min_size,capo->s->cover.max_size) )
        tag_ssize = "<image size=\"middle\">";
    else if( size_is_okay(34, capo->s->cover.min_size,capo->s->cover.max_size) )
        tag_ssize = "<image size=\"small\">";
    else if ( true || false )
        tag_ssize = "<image size=\"extralarge\">";

    // The (perhaps) result
    memCache_t * result = NULL;
    cache_list * r_list = NULL;

    int urlc = 0;

    char * find = capo->cache->data;
    while( (find = strstr(find+1, tag_ssize)) != NULL && urlc < capo->s->number && urlc < capo->s->plugmax)
    {
        char * end_tag = NULL;
        if( (end_tag = strstr(find, tag_esize)) != NULL)
        {
            char * url = NULL;
            if( (url = copy_value(find + strlen(tag_ssize), end_tag)) != NULL)
            {
                if(strcmp(url,"http://cdn.last.fm/flatness/catalogue/noimage/2/default_album_medium.png"))
                {
                    if(!r_list) r_list = DL_new_lst();

                    result = DL_init();
                    result->data = url;
                    result->size = end_tag - (find + strlen(tag_ssize));

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
    }
    return r_list;
}
