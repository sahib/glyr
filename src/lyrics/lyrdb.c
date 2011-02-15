#include <stdlib.h>
#include <string.h>

#include "lyrdb.h"

#include "../core.h"
#include "../core.h"
#include "../stringop.h"

#define LYRDB_URL "http://webservices.lyrdb.com/lookup.php?q=%artist%|%title%&for=match&agent=mpdbox"

const char * lyrics_lyrdb_url(glyr_settings_t * settings)
{
    return LYRDB_URL;
}

memCache_t * lyrics_lyrdb_parse(cb_object * capo)
{
    memCache_t * result = NULL;

    char *slash;
    if( (slash = strchr(capo->cache->data,'\\')) )
    {
        char * uID = copy_value(capo->cache->data,slash);
        if(uID)
        {
            char * lyr_url = strdup_printf("http://webservices.lyrdb.com/getlyr.php?q=%s",uID);
            if(lyr_url)
            {
                memCache_t * new_cache = download_single(lyr_url,capo->s);
                if(new_cache)
                {
                    char *buffer = malloc(new_cache->size+1);

                    int i;
                    for(i=0; i < new_cache->size; i++)
                        buffer[i] = (new_cache->data[i] == '\r') ? ' ' : new_cache->data[i];

                    buffer[i] = 0;

                    result = DL_init();
                    result->data = buffer;
                    result->size = i;
                    result->dsrc = strdup(lyr_url);

                    DL_free(new_cache);
                }
                free(lyr_url);
            }
            free(uID);
        }
    }
    return result;
}
