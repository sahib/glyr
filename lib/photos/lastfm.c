#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lastfm.h"

#include "../core.h"
#include "../core.h"
#include "../stringop.h"

// No choice on format, because everything smaller is pratctically unusable
#define SIZE_FO "<size name=\"extralarge\""
#define API_KEY "7199021d9c8fbae507bf77d0a88533d7"

#define URL_BEGIN "\">"
#define URL_ENDIN "</size>"

const char * photos_lastfm_url(glyr_settings_t * settings)
{
    return "http://ws.audioscrobbler.com/2.0/?method=artist.getimages&artist=%artist%&api_key="API_KEY;
}

cache_list * photos_lastfm_parse(cb_object * capo)
{
    char * root = capo->cache->data;
    cache_list * r_list = NULL;
    size_t urlc = 0;

    while ( (root = strstr(root,SIZE_FO)) != NULL && urlc < capo->s->number && urlc < capo->s->plugmax)
    {
        char * begin = strstr(root,URL_BEGIN);
        if(begin)
        {
            begin += strlen(URL_BEGIN);
            char * endin = strstr(begin,URL_ENDIN);
            if(endin)
            {
                char * urlb = copy_value(begin,endin);
                if(urlb)
                {
                    // init list if not done yet
                    if(!r_list) r_list = DL_new_lst();

                    memCache_t * cache = DL_init();
                    cache->data = urlb;
                    DL_add_to_list(r_list,cache);
                    urlc++;

                }
            }
        }

        root += strlen(SIZE_FO) - 1;
    }
    return r_list;
}
