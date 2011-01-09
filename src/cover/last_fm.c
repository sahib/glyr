#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "last_fm.h"

#include "../types.h"
#include "../core.h"


// Example snippet: 
/*
  What we get from last.fm: 
  
 <?xml version="1.0" encoding="UTF-8"?>
 <album artist="Equilibrium" title="Sagas">
     <reach>33152</reach>
     <url>http://www.last.fm/music/Equilibrium/Sagas</url>
     <releasedate>    8 Jul 2008, 00:00</releasedate>
     <coverart>
         <small>http://userserve-ak.last.fm/serve/34s/26963745.jpg</small>
         <medium>http://userserve-ak.last.fm/serve/64s/26963745.jpg</medium>
         <large>http://userserve-ak.last.fm/serve/126/26963745.jpg</large>
     </coverart>
*/


const char * cover_lastfm_url(void)
{
	return "http://ws.audioscrobbler.com/1.0/album/%artist%/%album%/info.xml"; 
}

memCache_t * cover_lastfm_parse(cb_object *capo)
{
	char *find, *endTag;
	
	// ~ max. size of last.fm:
	if(capo->min > 125) {
		return NULL;
	}

	// Handle size requirements (Default to large)
	char *tag_ssize = (capo->max == -1) ? "<large>"  : (capo->max < 40) ? "<small>"  : (capo->max < 70) ? "<medium>"  : "<large>"; 
	char *tag_esize = (capo->max == -1) ? "</large>" : (capo->max < 40) ? "</small>" : (capo->max < 70) ? "</medium>" : "</large>"; 

	if( (find = strcasestr(capo->cache->data, tag_ssize)) == NULL)
	{
		return NULL; 
	}

	nextTag(find); 

	if( (endTag = strcasestr(find, tag_esize) ) == NULL)
	{
		return NULL; 
	}

        size_t endTagLen = (size_t)(endTag - find); 	
	
	if(endTagLen == 0) return NULL;

	char *result_url = calloc(endTagLen+1, sizeof(char));
	strncpy(result_url, find, endTagLen);	
	result_url[endTagLen] = '\0';
	
	if(strcmp(result_url,"http://cdn.last.fm/flatness/catalogue/noimage/2/default_album_medium.png") != 0)
	 {
		memCache_t * result = DL_init();
		result->data = result_url;
		result->size = endTagLen;
		return result; 
	}

	if(result_url) free(result_url); 
	return NULL;
}
