#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>

#include "lyricswiki.h"

#include "../stringop.h"
#include "../core.h"
#include "../core.h"


const char * cover_lyricswiki_url(glyr_settings_t * sets)
{
    if(sets->cover.min_size < 500)
        return "http://lyrics.wikia.com/api.php?format=xml&action=query&list=allimages&aiprefix=%artist%";

    return NULL;
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

cache_list * cover_lyricswiki_parse(cb_object * capo)
{
    char * find=capo->cache->data;
    char * endTag   = NULL;

    int urlc = 0;
    cache_list * r_list = NULL;

    char *tmp = strreplace(capo->s->album," ","_");
    if(tmp)
    {
        char *_album = ascii_strdown(tmp,-1);
        if(_album)
        {
            // Go through all names and compare them with Levenshtein
            while(find && (find = strstr(find+1,IMG_TAG)) != NULL && urlc < capo->s->number && urlc < capo->s->plugmax)
            {
                // Find end & start of the name
                find  += strlen(IMG_TAG);
                endTag = strstr(find,END_TAG);

                if(endTag == NULL || endTag <= find)
                    continue;

                // Copy the name of the current album
                char * name = copy_value(find,endTag);
                if(name)
                {
                    // Remove .png / .jpg ...
                    int ll = endTag - find;
                    for(; ll; ll--)
                        if(name[ll] == '.')
                            name[ll] = '\0';

                    // Compare only lower case strings...
                    char * down_name = ascii_strdown(name,-1);
                    if(down_name)
                    {
                        // Allow max. 2 'typos'
                        if(levenshtein_strcmp(_album, down_name) < 4)
                        {
                            char *url_start = strstr(endTag,URL_MARKER);
                            if(url_start)
                            {
                                url_start += strlen(URL_MARKER);
                                char *url_end   = strstr(url_start,URL_END);
                                if(url_end)
                                {
                                    char * url = copy_value(url_start, url_end);
                                    if(url)
                                    {
                                        if(!r_list) r_list = DL_new_lst();

                                        memCache_t * result = DL_init();
                                        result->data = url;
                                        result->size = url_end - url_start;

                                        DL_add_to_list(r_list,result);
                                        urlc++;
                                    }

                                    url_end=NULL;
                                }

                                url_start=NULL;
                            }
                        }

                        // Get next img tag
                        find = strstr(endTag,"<img name=\"");
                        free(down_name);
                        down_name=NULL;
                    }
                }
                free(name);
                name=NULL;
            }
            free(_album);
            _album=NULL;
        }

        free(tmp);
        tmp=NULL;
    }
    return r_list;
}
