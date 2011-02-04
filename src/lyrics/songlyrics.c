#include <stdlib.h>
#include <string.h>

#include "songlyrics.h"

#include "../types.h"
#include "../core.h"
#include "../stringop.h"

const char * lyrics_songlyrics_url(glyr_settings_t * settings)
{
    if(settings && settings->artist && settings->title)
    {
        char * esc_a = ascii_strdown(settings->artist,-1);
        char * esc_t = ascii_strdown(settings->title,-1);

        if(esc_a && esc_t)
        {
            char * rep_a = strreplace(esc_a," ","-");
            char * rep_t = strreplace(esc_t," ","-");

            free(esc_a);
            free(esc_t);

            if(rep_a && rep_t)
            {
                char * url = strdup_printf("http://www.songlyrics.com/%s/%s-lyrics/",rep_a,rep_t);
                free(rep_a);
                free(rep_t);
                return url;
            }
        }
    }
    return NULL;
}

memCache_t * lyrics_songlyrics_parse(cb_object * capo)
{
    char * find_tag, * find_end;
    if( (find_tag = strstr(capo->cache->data,"<p id=\"songLyricsDiv\"")) != NULL)
    {
        if( (find_tag = strstr(find_tag,"&#")) != NULL)
        {
            if( (find_end = strstr(find_tag,"<p id=\"songLyricsSmallDiv\"")) != NULL)
            {
                size_t len = find_end - find_tag;
                char * buf = malloc(len+1);
                strncpy(buf,find_tag,len);
                buf[len] = '\0';

                char * brd = strreplace(buf,"<br />","");
                free(buf);

                memCache_t * result = DL_init();
                result->data = brd;
                result->size = strlen(brd);
                return result;
            }
        }
    }

    return NULL;
}
