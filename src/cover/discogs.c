#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>

#include "discogs.h"

#include "../types.h"
#include "../core.h"
#include "../stringop.h"


const char * cover_discogs_url(void)
{
	return "http://www.discogs.com/artist/%artist%?f=xml&api_key=adff651383";
}

// The discogs.com parser is a little more complicated... 
memCache_t * cover_discogs_parse(cb_object * capo)
{	
	char *raw_album  = ascii_strdown(capo->album,-1);
	char *find = capo->cache->data; 

	memCache_t * result = NULL; 

	if(raw_album == NULL) { 
		return NULL; 
	}

	while( (find = strstr(find,"<release id=")) != NULL && result == NULL) 
	{	
		// Get start of title tag
		char *t_s = strstr(find,"<title>"); 
		if(t_s == NULL) continue; 		

		// Get end of title tag
		char *t_e = strstr(t_s,"</title"); 
		if(t_e == NULL) continue; 

		// Get beginning of actual title
		t_s += strlen("<title>"); 

		// copy title	
		size_t len  = ABS(t_e - t_s); 
		char * title = malloc(len+1);
		strncpy(title,t_s,len); 
		title[len] = '\0'; 		
 
		// convert to lower case
		char *sc_title = NULL; 

		if(title!=NULL) {
			sc_title = ascii_strdown(title,-1); 		
			free(title); 
			title = NULL;
		} else  continue; 		

		// Compare via the levenshtein algorithm 
		if(levenshtein_strcmp(sc_title, raw_album) < 4) 
		{
			// nwo find the ID
			char * rev = strrstr_len( capo->cache->data,  "<release id=\"", t_s - capo->cache->data ) + strlen("<release id=\"");
		
			if(rev == NULL) {
				continue;						
			}	

			// Copy ID
			size_t ii=0;
			char IDbuf[16]; 
			while(*rev && *rev != '"') IDbuf[ii++] = *rev++;
			IDbuf[ii] = 0;  

			// Buld url to release page
			char *release_url = strdup_printf("http://www.discogs.com/release/%s?f=xml&api_key=adff651383",IDbuf); 
			if(release_url == NULL)
				continue; 

			// re-init cache
			if(capo->cache->size)	{
				DL_free(capo->cache); 
			}

			// Download release page
			capo->cache = download_single(release_url,1L);
			if(capo->cache != NULL) 
			{                       
				// Find the image url        
				char *URI, *URI_end; 

				// Restrict URL to type=primary 
				char *prim_tag = strstr(capo->cache->data,"type=\"primary\""); 
				if(prim_tag == NULL) continue; 
		
				// Check if size fits (only check width, covers are usally mostly quadratic..) 
				if(capo->min != -1 && capo->max != -1) 
				{
					char *width = strstr(prim_tag,"width=\"");
					if(width == NULL) continue; 
					width += strlen("width=\"");
		
					char wd_buf[9]; 
					int  wd_i = 0; 

					while(wd_i < 8 && width[wd_i] != '"') {
						wd_buf[wd_i] = width[wd_i];
						wd_i++;
					}
					wd_buf[wd_i] = '\0';	
			
					int c_size = atoi(wd_buf);
					if(c_size >= capo->min && c_size <= capo->max)
						 continue;
				}

				if( (URI = strcasestr(prim_tag, "uri=\"")) == NULL) 
				{
					continue; 
				}
				if( (URI_end = strcasestr(URI, "\" uri150=")) == NULL) 
				{
					continue;
				}

				// Copy url
				size_t ulen = (size_t)(URI_end - URI)-5;
				char* img_url = malloc(ulen+1); 
				strncpy(img_url, URI+5, ulen); 
				img_url[ulen] = 0;
				
				// Return url 
				result = DL_init();
				result->size = ulen;
				result->data = img_url;
			}
                                   
		}

		find = t_e;
	}

	// End of Parser	
	if(raw_album) {
		free(raw_album); 
		raw_album = NULL; 
	}
	return result;
}
