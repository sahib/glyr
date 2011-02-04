#include <stdlib.h>
#include <string.h>

#include "magistrix.h"

#include "../types.h"
#include "../core.h"
#include "../stringop.h"

#define AT_URL "http://lyrix.at/lyrics-search/s-%artist%,,%title%,,any,1321,0.html"

const char * lyrics_lyrixat_url(glyr_settings_t * settings)
{
    return AT_URL;
}

// The code ahead is a bit ugly..
memCache_t * lyrics_lyrixat_parse(cb_object * capo)
{
    /* lyrix.at does not offer any webservice -> use the searchfield to get some results */
    char *find, *endTag;
    const char* lyrURL_markup = "</a></td><td class='stitle'><a href='";

    if( (find = strstr(capo->cache->data,lyrURL_markup)) == NULL)
    {
        return NULL;
    }

    if( (endTag = strstr(find,"</a><br>Lyric<")) == NULL)
    {
        return NULL;
    }

    /* Now forget about the markup and go directly to the start of the url */
    find += strlen(lyrURL_markup);

    /* Get the url */
    size_t lyrURL_len =  (size_t)(endTag - find);
    char *lyrURL = malloc(lyrURL_len+1);
    int  i = 0;

    for(; i < lyrURL_len && find[i] && find[i] != '\''; i++)
    {
        lyrURL[i] = find[i];
    }

    lyrURL[i] = 0;
    char *real_url = strdup_printf("http://lyrix.at%s",lyrURL);
    if(lyrURL) free(lyrURL);

    if(real_url == NULL)
    {
        return NULL;
    }

    if(capo->cache && capo->cache->size)
    {
        DL_free(capo->cache);
    }

    capo->cache = download_single(real_url,1L);
    free(real_url);

    if(capo->cache != NULL)
    {
        char *lyr_start, *lyr_end;
        if( (lyr_start = strstr(capo->cache->data,"<div class='songtext' id='stextDIV'>")) == NULL)
        {
            return NULL;
        }

        if( (lyr_end = strstr(lyr_start,"<div>")) == NULL)
        {
            puts("Endtag is lacking existence");
            return NULL;
        }

        /* Part between lyr_start and lyr_end are lyrics that need to be 'translated' */
        size_t result_len = (size_t)(lyr_end - lyr_start);
        char* result = malloc(result_len + 1);

        int i = 0, x = 0;
        char tagflag = 0;

        for(; i < result_len; i++)
        {
            if(lyr_start[i] == '<')
            {
                tagflag = 1;
                continue;
            }


            if(lyr_start[i] == '>')
            {
                tagflag = 0;
                result[x] = '\n';
                continue;
            }


            if(tagflag == 0)
            {
                result[x++] = lyr_start[i];
            }

        }

        result[x] = 0;

        memCache_t * r_cache = DL_init();
        r_cache->data = result;
        r_cache->size = x;
        return r_cache;
    }

    return NULL;
}
