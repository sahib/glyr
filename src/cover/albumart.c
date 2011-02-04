#include <stdlib.h>
#include <string.h>

#include "amazon.h"

#include "../types.h"
#include "../core.h"
#include "../stringop.h"

const char * cover_albumart_url(glyr_settings_t * sets)
{
    return "http://www.albumart.org/index.php?srchkey=%artist%+%album%&itempage=1&newsearch=1&searchindex=Music";
}

#define AMZ "http://ecx.images-amazon.com/images/"

memCache_t * cover_albumart_parse(cb_object * capo)
{
    char * img_tag = strstr(capo->cache->data,"<div id=\"main\">");
    if(img_tag == NULL)
        return NULL;

    int size_it = 2, i = 0;
    if(capo->max < 450 && capo->max != -1 && capo->min < 160 && capo->min != -1)
        size_it = 1;

    for(; i < size_it; i++)
    {
        if((img_tag = strstr(img_tag,AMZ)) == NULL)
        {
            return NULL;
        }

        img_tag += strlen(AMZ);
    }

    char * img_end;
    if( (img_end  = strstr(img_tag,".jpg")) != NULL)
    {
        size_t img_len = img_end - img_tag;
        char * img_url = malloc(img_len+1);

        if(img_url)
        {
            strncpy(img_url,img_tag,img_len);
            img_url[img_len] = '\0';

            memCache_t * result = DL_init();
            result->data = strdup_printf(AMZ"%s.jpg",img_url);
            result->size = strlen(result->data);
            free(img_url);
            return result;
        }
    }

    return NULL;
}
