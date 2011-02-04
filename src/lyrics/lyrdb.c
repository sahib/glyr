#include <stdlib.h>
#include <string.h>

#include "lyrdb.h"

#include "../types.h"
#include "../core.h"
#include "../stringop.h"

#define LYRDB_URL "http://webservices.lyrdb.com/lookup.php?q=%artist%|%title%&for=match&agent=mpdbox"

const char * lyrics_lyrdb_url(glyr_settings_t * settings)
{
    return LYRDB_URL;
}

memCache_t * lyrics_lyrdb_parse(cb_object * capo)
{
    if(capo->cache == NULL || capo->cache->size == 0)
    {
        return NULL;
    }

    char *slash;
    if( (slash = strchr(capo->cache->data,'\\')) == NULL)
    {
        return NULL;
    }

    size_t uID_len = ABS(slash - capo->cache->data);
    char * uID     = malloc(uID_len + 1);
    strncpy(uID, capo->cache->data, uID_len);

    char * lyr_url = strdup_printf("http://webservices.lyrdb.com/getlyr.php?q=%s",uID);
    if(uID) free(uID);

    if(capo->cache && capo->cache->size)
    {
        DL_free(capo->cache);
    }

    capo->cache = download_single(lyr_url,1L);
    if(capo->cache != NULL)
    {
        char *result = malloc(capo->cache->size+1);

        int i;
        for(i=0; i < capo->cache->size; i++)
        {
            result[i] = (capo->cache->data[i] == '\r') ? ' ' : capo->cache->data[i];
        }

        strcrepl(result, 0x92, '\'');
        strcrepl(result, 0x93,  '"');
        result[i] = 0;

        memCache_t * r_cache = DL_init();
        r_cache->data = result;
        r_cache->size = i;

        return r_cache;
    }

    return NULL;

}
