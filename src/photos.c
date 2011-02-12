#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "photos.h"
#include "core.h"
#include "types.h"
#include "stringop.h"

#include "photos/flickr.h"
#include "photos/lastfm.h"

plugin_t photos_providers[] =
{
    {"lastfm", "l",  C_"last"C_R"."C_"fm",  false, {photos_lastfm_parse, photos_lastfm_url, false}},
    {"safe",   NULL, NULL,                  false, {NULL,              NULL,                false}},
    {"flickr", "f",  C_C"flick"C_R"r",      false, {photos_flickr_parse, photos_flickr_url, false}},
    {"unsafe", NULL, NULL,                  false, {NULL,              NULL,                false}},
    {NULL,     NULL, NULL,                  false, {NULL,              NULL,                false}}
};

plugin_t * glyr_get_photo_providers(void)
{
    return copy_table(photos_providers,sizeof(photos_providers));
}

static memCache_t * photo_callback(cb_object * capo)
{
    if(write_file(capo->name,capo->cache) == -1)
        fprintf(stderr,"Unable to write %s\n",capo->name);

    return NULL;
}

static const char * photo_finalize(memCache_t * result, glyr_settings_t * settings, const char * filename)
{
	size_t i=0,urlc = 0;
	cb_object * url_list = malloc(result->size * sizeof(cb_object));
	memset(url_list,0,result->size * sizeof(cb_object));

	// Used to track ptr where still memory is allocated
	char **urls_list_of_ptr = malloc(result->size * sizeof(char*));
	char **name_list_of_ptr = malloc(result->size * sizeof(char*));
	memset(urls_list_of_ptr,0,result->size * sizeof(char*));
	memset(name_list_of_ptr,0,result->size * sizeof(char*));

	char * l_url = NULL;
	char * c_url = result->data+1;

	for(; i < result->size && i < settings->photos.number; i++)
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

		    if(url)
		    {
			// copy url 
			strncpy(url,l_url,url_len);
			url[url_len] = '\0';

			// track this url
			urls_list_of_ptr[i] = url;

			char * name = strdup_printf("%s/%s_photo_%d.jpg",settings->save_path,settings->artist,i);
			if(name)
			{
			    // Keep an eye on this pointer so we can free it later
			    name_list_of_ptr[i] = name;

			    // Init "plugin" (this is actually a missuse of plugin_init())
			    // Init only when $artist_photo_$i does not exit yet, or if update is desired 
			    if(settings->update || access(name,R_OK))
				plugin_init(&url_list[urlc++],url,photo_callback,settings,name);
			}
		    }
		}
	    }

	    c_url++;
	}

	// This shall always return NULL, so no extra free is needed
	// The timeout is result->size as big because we need to download $result->size images
	// Arg  6,7,8 are not used also, they may not be empyt though 
	invoke(url_list, urlc, settings->parallel, settings->timeout * urlc, settings,"Downloading");

	// Free tracked pointers
	for(i = 0; i < result->size; i++)
	{
	    if(urls_list_of_ptr[i])
	    {
		free(urls_list_of_ptr[i]);
		urls_list_of_ptr[i]=NULL;
	    }

	    if(name_list_of_ptr[i])
	    {
		free(name_list_of_ptr[i]);
		name_list_of_ptr[i]=NULL;
	    }
	}

	// free all the rest
	if(url_list)
	{
	    free(url_list);
	    url_list=NULL;
	}
	if(urls_list_of_ptr)
	{
	    free(urls_list_of_ptr);
	    urls_list_of_ptr=NULL;
	}
	if(name_list_of_ptr)
	{
	    free(name_list_of_ptr);
	    name_list_of_ptr=NULL;
	}

	// return result
	char * rt_val = strdup(result->data);
	return rt_val;
}


char * get_photos(glyr_settings_t * settings)
{
    if (settings && settings->artist)
    	return (char*)register_and_execute(settings, NULL, photo_finalize);
    
    return NULL;
}
