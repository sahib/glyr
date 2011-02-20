#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "darklyrics.h"

#include "../core.h"
#include "../core.h"
#include "../stringop.h"

#define DARK_URL "http://darklyrics.com/lyrics/%artist%/%album%.html#1"

const char * lyrics_darklyrics_url(glyr_settings_t * settings)
{
    return DARK_URL;
}

cache_list * lyrics_darklyrics_parse(cb_object * capo)
{
    /* We have to search for the title in the data,
     * Therefore we convert everything to lowercase
     * to increase success chances.
     * Afterwards we copy from cache->data so we have
     * the correct case in the finaly lyrics
     * */

    cache_list * r_list = NULL;
    memCache_t * r_cache = NULL;

    char *searchstring = strdup_printf(". %s%s",capo->s->title,"</a></h3>");
    if(searchstring)
    {
        ascii_strdown_modify(searchstring,-1);
        char *cache_copy = strdup(capo->cache->data);
        if(cache_copy)
        {
            ascii_strdown_modify(cache_copy,-1);
            char *head = strstr(cache_copy,searchstring);
            if(head)
            {
                char *foot  = strstr(head, "<a name=");
                if(foot)
                {
                    char * html_code = copy_value(head,foot);
                    if(html_code)
                    {
                        r_cache = DL_init();
                        r_cache->data = strreplace(html_code,"<br />","");
                        r_cache->size = strlen(r_cache->data);
                        r_cache->dsrc = strdup(capo->url);
                        free(html_code);
                    }
                }
            }
            free(cache_copy);
        }
        free(searchstring);
    }

    if(r_cache)
    {
	r_list = DL_new_lst();
	DL_add_to_list(r_list,r_cache);
    }
    return r_list;
}
