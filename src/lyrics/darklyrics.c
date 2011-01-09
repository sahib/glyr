#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "darklyrics.h"

#include "../types.h"
#include "../core.h"
#include "../stringop.h"

#define DARK_URL "http://darklyrics.com/lyrics/%artist%/%album%.html#1"

const char * lyrics_darklyrics_url(void)
{
	return DARK_URL;
}

memCache_t * lyrics_darklyrics_parse(cb_object * capo)
{
	/* We have to search for the title in the data, 
	 * Therefore we convert everything to lowercase 
	 * to increase success chances. 
	 * Afterwards we copy from cache->data so we have 
	 * the correct case in the finaly lyrics 
	 * */
	 
	if(capo->album == NULL)
	{
		return NULL;
	}
	 
	char *searchstring = ascii_strdown(strdup_printf(". %s%s",capo->title,"</a></h3>"),-1); 
	char *cache_copy   = ascii_strdown(strdup(capo->cache->data),-1); 
	char *head = strstr(cache_copy,searchstring); 
	int len    = strlen(searchstring);

	free(searchstring);
	 
	if(!head)
	{
		return NULL; 
	}
	
	char *foot  = strstr(head, "<a name=");
	if(!foot) 
	{
		return NULL; 
	}

	int pos1  = (int)ABS(cache_copy - head);
	int pos2  = (int)ABS(cache_copy	- foot);
	free(cache_copy);
	
	char *lyrbuf = calloc((pos2-pos1)+2, sizeof(char));
	int i = (len+pos1), j = 0; 
	bool doset = true;
	
	for(;i < pos2 && capo->cache->data[i];i++)
	{
		    if(capo->cache->data[i] == '<') doset = false;
			if(doset && capo->cache->data[i] != '\t' && capo->cache->data[i] != '\r') lyrbuf[j++] = capo->cache->data[i];
			if(capo->cache->data[i] == '>') doset = true;
	}

	lyrbuf[j] = '\0';
	
	memCache_t * r_cache = DL_init();
	r_cache->data = lyrbuf;
	r_cache->size = j;
	return r_cache;
}
