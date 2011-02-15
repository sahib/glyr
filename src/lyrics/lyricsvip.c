#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lyricsvip.h"

#include "../core.h"
#include "../core.h"
#include "../stringop.h"

#define LV_URL "http://www.lyricsvip.com/%s/%s-Lyrics.html"

const char * lyrics_lyricsvip_url(glyr_settings_t * settings)
{
    char * result = NULL;
    char * artist_clean = strreplace(settings->artist, " ", "-");
    if(artist_clean)
    {
        char * title_clean =  strreplace(settings->title, " ", "-");
        if(title_clean)
        {
            result = strdup_printf(LV_URL, artist_clean, title_clean);
            free(title_clean);
        }
        free(artist_clean);
    }
    return result;
}

memCache_t * lyrics_lyricsvip_parse(cb_object *capo)
{
    char * start = NULL;
    char * end = NULL;
    char * content = NULL;

    if ((start = strstr(capo->cache->data,"<table class=\"tbl0\">")) == NULL)
    {
        return NULL;
    }

    if ((end = strstr(start,"</table>")) == NULL)
    {
        return NULL;
    }

    if (ABS(end-start) <= 0)
    {
        return NULL;
    }

    *(end) = 0;

    content = strreplace(start,"<br />",NULL);

    memCache_t * r_cache = DL_init();
    r_cache->data = content;
    r_cache->size = strlen(content);
    r_cache->dsrc = strdup(capo->url);
    return r_cache;
}

