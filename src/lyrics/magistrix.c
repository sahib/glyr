#include <stdlib.h>
#include <string.h>

#include "magistrix.h"

#include "../types.h"
#include "../core.h"
#include "../stringop.h"

#define MG_URL "http://www.magistrix.de/lyrics/search?q=%artist%+%title%"

const char * lyrics_magistrix_url(void)
{
	return MG_URL;
}

memCache_t * lyrics_magistrix_parse (cb_object * capo)
{
	/* By issuing the base url we either get  a ready-to-parse lyricpage or some search results */
	char *find, *endTag;

    if( strstr(capo->cache->data,"<div class='empty_collection'>") )
    {
        /* We're on the 'no songtext' page */
        return NULL;
    }
    else if( (find = strstr(capo->cache->data,"<table cellspacing='0' class='box1'>")) != NULL)
    {
        int i = 0;
        char *real_url = NULL;

        for(;i<4;i++) nextTag(find);

        if(find == NULL || *find == 0)
        {
            return NULL;
        }

        while(*find && *find != '"')
            { find++; }

        find++;
        if( (endTag = strstr(find,"\" class")) == NULL)
        {
            return NULL;
        }

        size_t mini_url_len = ABS(endTag - find);
        char *mini_url = malloc(mini_url_len + 1);
        strncpy(mini_url, find, mini_url_len);
        mini_url[mini_url_len] = 0;

        real_url = strdup_printf("http://www.magistrix.de%s",mini_url);
        if(mini_url) free(mini_url);

        if(capo->cache && capo->cache->size)
        {
            DL_free(capo->cache);
        }

        capo->cache = download_single(real_url,1L);
        if(capo->cache == NULL)
        {
            return NULL;
        }
    }

    /* At this point we have the lyrics page in cache (hopefully) - do some parsing */
    if( (endTag =strstr(capo->cache->data,"<fb:like colorscheme='dark'></fb:like>")))
    {
        size_t search_len = ABS(endTag - capo->cache->data);
        char *start = strrstr_len (capo->cache->data,"</div>", search_len);
        if(start == NULL)
        {
            printf("No start..");
            return NULL;
        }

        size_t i = 0, x = 0;
        size_t lyr_len = ABS(endTag - start);

        char tagflag = 0;
        char donline = 1;

        char *lyrics = malloc(lyr_len + 1);
        for(; i < lyr_len; i++)
        {
            if(start[i] == '<')
            {
                tagflag = 1;
                continue;
            }

            if(start[i] == '>')
            {
                tagflag = 0;
                if(donline)
                {
                    lyrics[x] = '\n';
                    donline = 1;
                }
                continue;
            }

            if(tagflag == 0)
            {
                lyrics[x++] = start[i];
                donline = 1;
            }
        }

        /* Set Nullbyte */
        lyrics[x] = 0;
        memCache_t * r_cache = DL_init();
        r_cache->data = lyrics;
        r_cache->size  = x;
        return r_cache;
    }

	return NULL;

}
