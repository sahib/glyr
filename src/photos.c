#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "photos.h"
#include "core.h"
#include "types.h"
#include "stringop.h"

#include "photos/flickr.h"

sk_pair_t photos_providers[] =
{
    {"flickr", "f",  C_C"flick"C_R"r", false, {photos_flickr_parse, photos_flickr_url, false}},
    {"safe",   NULL, NULL,             false, {NULL,              NULL,                false}},
    {NULL,     NULL, NULL,             false, {NULL,              NULL,                false}}
};

sk_pair_t * glyr_get_photo_providers(void)
{ 
    return photos_providers;
}

static memCache_t * photo_callback(cb_object * capo)
{
	if(write_file(capo->name,capo->cache) == -1)
	{
		puts(" ");
	} 
	return NULL;	
}

static const char * finalize(cb_object * plugin_list, size_t it,  const char *filename, glyr_settings_t * settings)
{
    // Now do the actual work after all that plugin preparing..
    memCache_t *result = invoke(plugin_list, it, COVER_MAX_PLUGIN, DEFAULT_TIMEOUT, DEFAULT_REDIRECTS, settings->artist, settings->album,"unused",NULL);

    if (result != NULL && result->data != NULL && filename)
    {
	size_t i=0,urlc = 0;
	cb_object * url_list = malloc(result->size * sizeof(cb_object));
	memset(url_list,0,result->size * sizeof(cb_object));

	char * l_url = NULL;
	char * c_url = result->data+1;

	for(; i < result->size; i++)
	{
		l_url = c_url;
		c_url = strstr(l_url,"\n");

		if(l_url)
		{
			size_t  url_len;
			if(c_url)
				url_len = c_url - l_url;
			else 
				url_len = strlen(l_url);				

			if(url_len > 0)
			{
				char * url = malloc(url_len+1);

				if(url && *url)
				{
					strncpy(url,l_url,url_len);
					url[url_len] = '\0';
				
					char * name = strdup_printf("%s/%s_photo_%d.jpg",settings->save_path,settings->artist,i);

					if(name)
					{
						plugin_init(&url_list[urlc++],url,photo_callback,0,0,name);
					}
				}
			}
		}
		
		c_url++;
	}

	invoke(url_list, urlc, result->size, DEFAULT_TIMEOUT * urlc, DEFAULT_TIMEOUT , "Not","important","at all","Downloading");

	for(i = 0; i < result->size; i++)
	{
		if(url_list[i].url)
		    free(url_list[i].url);

		if(url_list[i].name)
		    free((char*)url_list[i].name);
	}

	if(url_list)
    	    free(url_list);	
    }

    DL_free(result);
    return NULL;
}


char * get_photos(glyr_settings_t * settings, const char * filename)
{
    if (settings == NULL || settings->artist == NULL)
    {
        fprintf(stderr,C_R"(!) "C_"<%s> is empty, but it is needed for photoDL!\n","artist");
        return NULL;
    }

    if(register_and_execute(settings, filename, finalize) != NULL)
    {
        return strdup(filename);
    }
    else
    {
        return NULL;
    }
}
