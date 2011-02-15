#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lastfm.h"

#include "../core.h"
#include "../core.h"
#include "../stringop.h"

#define SUMMARY_BEGIN "<summary><![CDATA["
#define SUMMARY_ENDIN "]]></summary>"

#define CONTENT_BEGIN "<content><![CDATA["
#define CONTENT_ENDIN "]]></content>"

const char * adescr_lastfm_url(glyr_settings_t * s)
{
	return "http://ws.audioscrobbler.com/2.0/?method=artist.getinfo&artist=%artist%&autocorrect=0&api_key=7199021d9c8fbae507bf77d0a88533d7";
}

static char * fix_crappy_lastfm_txt(const char * txt)
{
	char * result = NULL;
	if(txt)
	{
		char * no_html = remove_html_tags_from_string(txt,strlen(txt));
		if(no_html)
		{
			char * no_unicode = strip_html_unicode(no_html);
			if(no_unicode)
			{
				char * no_unicode_at_all = unescape_html_UTF8(no_unicode);
				if(no_unicode_at_all)
				{
					char * trim_buf = calloc(strlen(no_unicode_at_all)+1,sizeof(char));
					if(trim_buf)
					{
						trim_copy(no_unicode_at_all,trim_buf);
						result = trim_buf;
					}
					free(no_unicode_at_all);
				}
				free(no_unicode);
			}
			free(no_html);
		}
	}
	return result;
}

memCache_t * adescr_lastfm_parse(cb_object * capo)
{
	memCache_t * result = NULL;

	char * short_desc = copy_value(strstr(capo->cache->data,SUMMARY_BEGIN)+strlen(SUMMARY_BEGIN),strstr(capo->cache->data,SUMMARY_ENDIN));
	if(short_desc)
	{
		char * content = copy_value(strstr(capo->cache->data,CONTENT_BEGIN)+strlen(CONTENT_BEGIN),strstr(capo->cache->data,CONTENT_ENDIN));
		if(content)
		{
			char * assembled = strdup_printf("\n%s\n__split__\n%s\n",short_desc,content);
			if(assembled)
			{

				char * corrected = fix_crappy_lastfm_txt(assembled);
				if(corrected)
				{
					result = DL_init();
					result->data = corrected;
					result->size = strlen(corrected);
					result->dsrc = strdup(capo->url);
				}
				free(assembled);
				assembled=NULL;
			}
			free(content);	
			content=NULL;
		}
		free(short_desc);
		short_desc=NULL;
	}
	return result;
}
