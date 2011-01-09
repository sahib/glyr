#include <stdlib.h>
#include <string.h>

#include "lyricswiki.h"

#include "../types.h"
#include "../core.h"
#include "../stringop.h"

#define LW_URL "http://lyrics.wikia.com/api.php?action=lyrics&fmt=xml&func=getSong&artist=%artist%&song=%title%"

const char * lyrics_lyricswiki_url(void)
{
	return LW_URL;
}

memCache_t * lyrics_lyricswiki_parse(cb_object * capo)
{
  char *find, *endTag; 
  
  if( (find = strstr(capo->cache->data,"<url>")) == NULL) 
  {
	  return NULL; 
  }
  
  nextTag(find);
  
  while( (endTag = strstr(find, "</url>")) == NULL) 
  {
	  return NULL; 
  }
  
  size_t len = ABS(endTag - find);
  char *wiki_page_url = malloc(len + 1);
  strncpy(wiki_page_url, find, len);
  wiki_page_url[len] = 0;  
  
  if(capo->cache && capo->cache->size) 
  {
	  DL_free(capo->cache);
  }
  
  capo->cache = download_single(wiki_page_url, 1L); 
  if(capo->cache != NULL) 
  {
	  char *lyr_begin, *lyr_end; 
	  if( (lyr_begin = strstr(capo->cache->data, "'17'/></a></div>")) == NULL) 
	  {
		  return NULL; 
	  }
	  
	  nextTag(lyr_begin);
	  nextTag(lyr_begin);
	  nextTag(lyr_begin);
	
	  if(lyr_begin == NULL)
	  {
		  return NULL;
	  }
	  
	  if( (lyr_end = strstr(lyr_begin, "<!--")) == NULL) 
	  {
		  return NULL; 
	  }
	  
	  size_t lyr_len = ABS(lyr_end - lyr_begin); 
	  char *lyr_utf8 = malloc(lyr_len + 1); 
	  strncpy(lyr_utf8, lyr_begin, lyr_len); 
	  
	  char *result = unescape_html_UTF8(lyr_utf8);
	  if(lyr_utf8) free(lyr_utf8);

	  memCache_t * r_cache = DL_init();
	  r_cache->data = result;
	  r_cache->size = lyr_len;
	  return r_cache;
  }
  
  return NULL;	
}
