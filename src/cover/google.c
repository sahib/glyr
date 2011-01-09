#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>

#include "google.h"

#include "../types.h"
#include "../core.h"

#define FIRST_RESULT "<td align=left valign=bottom width=23% style=\"padding-top:1px\"><a href=/imgres?imgurl="
#define END_OF_URL   "&imgrefurl="

const char * cover_google_url(int min, int max)
{
	// Only use the 'large' option if really set high (>1000), large is usually verrrry incorrrect.
	     if(min == -1)               {  return "http://www.google.de/images?q=%artist%+%album%+album&safe=off&tbs=isch:1,iar:s,islt:qsvga"; }
	else if(min <  50 && max <= 150) { return "http://www.google.de/images?q=%artist%+%album%+album&safe=off&tbs=isch:1,iar:s,isz:i";       }
	else if(min < 150 && max <= 300) { return "http://www.google.de/images?q=%artist%+%album%+album&safe=off&tbs=isch:1,iar:s,isz:s";       }
	else if(min < 300 && max <= 1000){ return "http://www.google.de/images?q=%artist%+%album%+album&safe=off&tbs=isch:1,iar:s,islt:qsvga";  }
	else	                         { return "http://www.google.de/images?q=%artist%+%album%+album&safe=off&tbs=isch:1,iar:s,islt:vga";    }
}

memCache_t * cover_google_parse(cb_object * capo) 
{
	// we blindly take the first result 
	// actually we have not much of a choice (no, not Nr.42)
	char * find, * end_of_url;

	if( (find =  strstr(capo->cache->data,FIRST_RESULT)) == NULL) 
	{
		return NULL; 
	}
	
	find += strlen(FIRST_RESULT); 
	if(find == NULL || (end_of_url = strstr(find, END_OF_URL)) == NULL)
	{
		return NULL;
	}

	size_t len = end_of_url - find; 
	char *url = malloc(len+1);
	strncpy(url,find,len);
	url[len] = 0;

	memCache_t * result = DL_init();
	if(result != NULL)
	{
		result->data = url;
		result->size = len;
		return result;
	}
	
	return NULL;
}
