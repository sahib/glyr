#include <stdlib.h>
#include <string.h>

#include "amazon.h"

#include "../types.h"
#include "../core.h"
#include "../stringop.h"

const char * cover_albumart_url(glyr_settings_t * sets)
{
    if(sets->cover.min_size < 160)
    return "http://www.albumart.org/index.php?srchkey=%artist%+%album%&itempage=1&newsearch=1&searchindex=Music";

    return NULL;
}

#define AMZ "http://ecx.images-amazon.com/images/"

memCache_t * cover_albumart_parse(cb_object * capo)
{
    memCache_t * result = NULL;

    char * img_tag = strstr(capo->cache->data,"<div id=\"main\">");
    if(img_tag)
    {
	    int size_it = 2, i = 0;
	    if(capo->max < 450)
		size_it = 1;

	    for(; i < size_it; i++)
	    {
		if((img_tag = strstr(img_tag,AMZ)) == NULL)
		    return NULL;

		img_tag += strlen(AMZ);
	    }

	    char * img_end;
	    if( (img_end  = strstr(img_tag,".jpg")) != NULL)
	    {
		char * img_url = copy_value(img_tag,img_end);
		if(img_url)
		{
		    result = DL_init();
		    result->data = strdup_printf(AMZ"%s.jpg",img_url);
		    result->size = strlen(result->data);

		    free(img_url);
		    img_url=NULL;	
		}
	    }
    }
    return result;
}
