#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "flickr.h"

#include "../types.h"
#include "../core.h"
#include "../stringop.h"

#define LINE_BEGIN "<photo id="
#define LINE_ENDIN "/>"

const char * photos_flickr_url(glyr_settings_t * settings)
{
	return "http://api.flickr.com/services/rest/"
	       "?method=flickr.photos.search&"
               "api_key=b5af0c3230fb478d53b20835223d57a4&"
               "tags=%artist%&"
               "content_type=1&"
               "media=photos&"
               "is_gallery=true&"
               "per_page=%album%";
}

static char * get_field_by_name(const char * string, const char * name)
{
	if(string)
	{
		char * find = strstr(string,name);
		if(find)
		{
			find += strlen(name);

			if(*find == '"') 
			    find ++;

			char * end = strstr(find,"\"");
			if(end)
			{
				size_t len = end - find;
				char * buf = malloc(len+1);
				strncpy(buf,find,len);
				buf[len] = '\0';
				return buf;
			}
		}
	}
	return NULL;
}

memCache_t * photos_flickr_parse(cb_object * capo)
{
	// Needed: ID,secret,server,farm
	char * ph_begin = capo->cache->data;
	char * result   = "";		

	size_t urlc = 0;

	while( (ph_begin=strstr(ph_begin,LINE_BEGIN)) != NULL)
	{
		ph_begin++;

		char * ph_end = strstr(ph_begin,LINE_ENDIN);
		if(ph_end)
		{
			size_t lineln = ph_end - ph_begin;
			char * linebf = malloc(lineln+1);

			if(linebf)
			{
				strncpy(linebf,ph_begin,lineln);
				linebf[lineln] = '\0';
	
	
				char * ID = get_field_by_name(linebf, "id=");
				char * SC = get_field_by_name(linebf, "secret=");
				char * SV = get_field_by_name(linebf, "server=");
				char * FR = get_field_by_name(linebf, "farm=");

				free(linebf);

			//	printf("http://farm%s.static.flickr.com/%s/%s_%s.jpg",FR,SV,ID,SC);
				result = strdup_printf("%s\nhttp://farm%s.static.flickr.com/%s/%s_%s.jpg",result,FR,SV,ID,SC);

				if(ID) 
				    free(ID);
				if(SC)
				    free(SC);
				if(SV)
				    free(SV);
				if(FR)
				    free(FR);

				urlc++;
			}
		}
		
	}

	if(result == NULL)
            return NULL;	

	memCache_t * result_cache = DL_init();
	result_cache->data = result;
	result_cache->size = urlc;

	return result_cache;
}
