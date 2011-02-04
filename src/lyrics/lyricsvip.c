#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lyricsvip.h"

#include "../types.h"
#include "../core.h"
#include "../stringop.h"

#define LV_URL "http://www.lyricsvip.com/%s/%s-Lyrics.html"

const char * lyrics_lyricsvip_url(glyr_settings_t * settings)
{
    char * artist_clean = strreplace(settings->artist, " ", "-");
    char * title_clean =  strreplace(settings->title, " ", "-");

    if(artist_clean && title_clean)
    {
        char * url = strdup_printf(LV_URL, artist_clean, title_clean);

        free(artist_clean);
        free(title_clean);
        return url;
    }

    return NULL;
}

memCache_t * lyrics_lyricsvip_parse(cb_object *capo)
{
    char * start = NULL;
    char * end = NULL;
    char * content = NULL;
    size_t len = 0;

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

    len = strlen(start);
    content = strreplace(start,"<br />","");


    memCache_t * r_cache = DL_init();
    r_cache->data = content;
    r_cache->size = strlen(content);
    return r_cache;
}

