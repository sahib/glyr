#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>

#include "lyricswiki.h"

#include "../stringop.h"
#include "../types.h"
#include "../core.h"


const char * cover_lyricswiki_url(glyr_settings_t * sets)
{
    return "http://lyrics.wikia.com/api.php?format=xml&action=query&list=allimages&aiprefix=%artist%";
}


/**

<img name="Axxis_-_Access_All_Areas.jpg"
	timestamp="2010-08-03T17:05:20Z"
	url="http://images.wikia.com/lyricwiki/images/f/f9/Axxis_-_Access_All_Areas.jpg"
	descriptionurl="http://lyrics.wikia.com/File:Axxis_-_Access_All_Areas.jpg"
/>

**/

#define IMG_TAG "_-_"
#define END_TAG "\" timestamp=\""
#define URL_MARKER "url=\""
#define URL_END "\" descriptionurl="

memCache_t * cover_lyricswiki_parse(cb_object * capo)
{
    char * find=capo->cache->data;
    char * endTag   = NULL;
    char * c_return = NULL;
    size_t c_size   = 0;

    char *tmp = strreplace(capo->album," ","_");
    if(tmp == NULL)
    {
        return NULL;
    }

    char *_album = ascii_strdown(tmp,-1);
    free(tmp);

    if(_album == NULL)
    {
        return NULL;
    }

    // Go through all names and compare them with Levenshtein
    while(find && (find = strstr(find,IMG_TAG)) != NULL && c_return == NULL)
    {
        // Find end & start of the name
        find  += strlen(IMG_TAG);
        endTag = strcasestr(find,END_TAG);

        if(endTag == NULL || endTag <= find)
            continue;

        // Copy the name of the current album
        size_t len = (endTag - find);
        char *name = malloc(len+1);
        strncpy(name,find,len);
        name[len] = 0;

        // Remove .png / .jpg ...
        int ll = len;
        for(; ll != 0; ll--)
        {
            if(name[ll] == '.')
            {
                name[ll] = '\0';
            }
        }

        // Compare only lower case strings...
        char * down_name = ascii_strdown(name,-1);

        // Allow max. 2 'typos'
        if(levenshtein_strcmp(_album, down_name) < 4)
        {
            char *url_start = strstr(endTag,URL_MARKER);
            url_start += strlen(URL_MARKER);

            char *url_end   = strstr(url_start,URL_END);

            size_t url_len = (url_end - url_start);
            char *url = malloc(url_len + 1);
            strncpy(url,url_start,url_len);
            url[url_len] = 0;

            c_return = url;
            c_size = url_len;
        }

        if(name) free(name);

        // Get next img tag
        find = strstr(endTag,"<img name=\"");
    }

    free(_album);

    if(c_return != NULL)
    {
        memCache_t *c_cache = DL_init();
        c_cache->data = c_return;
        c_cache->size = c_size;
        return c_cache;
    }
    return NULL;
}
