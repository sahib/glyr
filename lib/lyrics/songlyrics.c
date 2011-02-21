#include <stdlib.h>
#include <string.h>

#include "songlyrics.h"

#include "../core.h"
#include "../core.h"
#include "../stringop.h"

const char * lyrics_songlyrics_url(glyr_settings_t * settings)
{
    char * url = NULL;
    if(settings && settings->artist && settings->title)
    {
        char * esc_a = ascii_strdown(settings->artist,-1);
        if(esc_a)
        {
            char * esc_t = ascii_strdown(settings->title,-1);
            if(esc_t)
            {
                char * rep_a = strreplace(esc_a," ","-");
                if(rep_a)
                {
                    char * rep_t = strreplace(esc_t," ","-");
                    if(rep_t)
                    {
                        url = strdup_printf("http://www.songlyrics.com/%s/%s-lyrics/",rep_a,rep_t);
                        free(rep_t);
                    }
                    free(rep_a);
                }
                free(esc_t);
            }
            free(esc_a);
        }
    }
    return url;
}

cache_list * lyrics_songlyrics_parse(cb_object * capo)
{
    memCache_t * result = NULL;
    cache_list * r_list = NULL;
    char * find_tag, * find_end;
    if( (find_tag = strstr(capo->cache->data,"<p id=\"songLyricsDiv\"")) != NULL)
    {
        if( (find_tag = strstr(find_tag,"&#")) != NULL)
        {
            if( (find_end = strstr(find_tag,"<p id=\"songLyricsSmallDiv\"")) != NULL)
            {
                char * buf = copy_value(find_tag,find_end);
                if(buf)
                {
                    char * brd = strreplace(buf,"<br />","");
                    if(brd)
                    {
                        result = DL_init();
                        result->data = brd;
                        result->size = strlen(brd);
                        result->dsrc = strdup(capo->url);
                    }
                    free(buf);
                }
            }
        }
    }
    if(result)
    {
        r_list = DL_new_lst();
        DL_add_to_list(r_list, result);
    }
    return r_list;
}
