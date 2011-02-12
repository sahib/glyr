#include <stdlib.h>
#include <string.h>

#include "magistrix.h"

#include "../types.h"
#include "../core.h"
#include "../stringop.h"

#define MG_URL "http://www.magistrix.de/lyrics/search?q=%artist%+%title%"
#define LV_MAX_DIST 4

const char * lyrics_magistrix_url(glyr_settings_t * settings)
{
    return MG_URL;
}

memCache_t * parse_lyric_page(const char * buffer)
{
	memCache_t * result = NULL;
	if(buffer)
	{
		char * begin = strstr(buffer,"<div id='songtext'>");
		if(begin)
		{
			begin = strstr(begin,"</div>");
			{	
				char * end = strstr(begin+1,"</div>");
				if(end)
				{
					char * lyr = copy_value(begin,end);
					if(lyr)
					{
						result = DL_init();
						result->data = strreplace(lyr,"<br />","");
						result->size = strlen(result->data);
						free(lyr);
					}
				}
			}
		}
	}
	return result;	
}

static bool approve_content(char * content, const char * compare)
{	
	if(compare)
	{	
		char * tmp = strdup(compare);
		if(levenshtein_strcmp(ascii_strdown_modify(content,-1),ascii_strdown_modify(tmp,-1)) <= LV_MAX_DIST)
		{
			free(tmp);
			return true;
		}
		free(tmp);
	}
	return false;
}

#define ARTIST_BEGIN "class=\"artistIcon bgMove\">"
#define TITLE_BEGIN "\" class=\"lyricIcon bgMove\">"
#define URL_BEGIN   "<a href=\""
#define SEARCH_END   "</a>"

memCache_t * lyrics_magistrix_parse (cb_object * capo)
{
    memCache_t * result=NULL;
    if( strstr(capo->cache->data,"<div class='empty_collection'>") == NULL) // No songtext page?
    {
	if( strstr(capo->cache->data,"<title>Songtext-Suche</title>") == NULL) // Are we not on the search result page?
	{
		result = parse_lyric_page(capo->cache->data);
	}
	else
	{
		char * node = capo->cache->data;
		while( (node = strstr(node+1,"<tr class='topLine'>")) && !result )
		{
			char * artist = copy_value(strstr(node,ARTIST_BEGIN)+strlen(ARTIST_BEGIN),strstr(node,"</a>"));
			if(artist)
			{
				if(approve_content(artist,capo->artist))
				{
					char * title_begin = strstr(node,TITLE_BEGIN);
					if(title_begin)
					{
						char * title = copy_value(title_begin+strlen(TITLE_BEGIN),strstr(title_begin,"</a>"));
						if(title)
						{
							if(approve_content(title,capo->title))
							{
								char * url_begin = strstr(node,URL_BEGIN);
								if(url_begin)
								{
									url_begin = strstr(url_begin+1,URL_BEGIN);
									if(url_begin)
									{
										char * url = copy_value(url_begin+strlen(URL_BEGIN),title_begin);
										if(url)
										{
											char * dl_url = strdup_printf("www.magistrix.de%s",url);
											if(dl_url)
											{
												memCache_t * dl_cache = download_single(dl_url,1L);
												if(dl_cache)
												{
													result = parse_lyric_page(dl_cache->data);
													DL_free(dl_cache);
												}
											}
											free(url);
										}
									}
								}
							}
							free(title);
						}
					}
				}
				free(artist);
			}
		}

	}
    }
   
    return result;

}
