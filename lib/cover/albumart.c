#include <stdlib.h>
#include <string.h>

#include "amazon.h"

#include "../core.h"
#include "../core.h"
#include "../stringop.h"

const char * cover_albumart_url(glyr_settings_t * sets)
{
    if(sets->cover.min_size <= 500 || sets->cover.min_size == -1)
        return "http://www.albumart.org/index.php?srchkey=%artist%+%album%&itempage=1&newsearch=1&searchindex=Music";

    return NULL;
}

#define AMZ "http://ecx.images-amazon.com/images/"

cache_list * cover_albumart_parse(cb_object * capo)
{
    cache_list * r_list = NULL;

    char * node = strstr(capo->cache->data,"<div id=\"main\">");
    if(node)
    {
        size_t size_it = 2;

        if(capo->s->cover.max_size < 450 && capo->s->cover.max_size != -1 && capo->s->cover.min_size < 160)
        {
            size_it = 1;
        }

        size_t urlc = 0;
        while( (node = strstr(node+1,"<li><div style=\"")) && urlc < capo->s->number && urlc < capo->s->plugmax)
        {
            size_t i = 0;
            char * img_tag = node;
            char * img_end = NULL;

            for(i = 0; i < size_it; i++, img_tag += strlen(AMZ))
                if((img_tag = strstr(img_tag,AMZ)) == NULL)
                    break;

            if( (img_end  = strstr(img_tag,".jpg")) != NULL)
            {
                char * img_url = copy_value(img_tag,img_end);
                if(img_url)
                {
                    if(!r_list) r_list = DL_new_lst();

                    memCache_t * result = DL_init();
                    result->data = strdup_printf(AMZ"%s.jpg",img_url);
                    result->size = strlen(result->data);

                    DL_add_to_list(r_list,result);

                    urlc++;

                    free(img_url);
                }
            }
        }
    }
    return r_list;
}
