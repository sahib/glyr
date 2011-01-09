#define _GNU_SOURCE

// standard
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// own header
#include "metrolyrics.h"

// Include types, DL*() and
// extended string lib
#include "../types.h"
#include "../core.h"
#include "../stringop.h"

// Search URL
#define ML_URL "http://www.metrolyrics.com/search.php?search=%artist%+%title%&category=artisttitle"
#define LEVEN_TOLERANCE 4


// Just return URL
const char * lyrics_metrolyrics_url(void)
{
    return ML_URL;
}

// Compare the entry via the levenstehin algorithm 
// This ensures that no wrong lyrics is downloaded
bool check_ref(const char * cur_ref, const char * artist, const char * title)
{
	char * my_ref = (char*)cur_ref;

	if(my_ref[0] == '/')
	{
		my_ref++;
	}
	
	char * p_artist = strcasestr(my_ref,"-lyrics-") + strlen("-lyrics-");
	if(p_artist == NULL)
	{
		return false;
	}
	
	size_t p_artlen = strlen(p_artist);
	size_t p_titlen = p_artist - my_ref;
	
	if(p_artlen < 5 || p_titlen < 1)
	{
		return false;
	}	

	p_artist[p_artlen - 5] = 0;

	char * actual_artist = strreplace(p_artist,"-"," ");
	if( levenshtein_strcmp(actual_artist,artist) > LEVEN_TOLERANCE)
	{
		free(actual_artist);
		return false;
	}

	char * tmp_title  = malloc(p_titlen + 1);
	strncpy(tmp_title, my_ref, p_titlen);
	tmp_title[p_titlen-strlen("-lyrics-")] = '\0';	
	char * actual_title = strreplace(tmp_title,"-"," ");
	free(tmp_title);

	if( levenshtein_strcmp(actual_title,title) > LEVEN_TOLERANCE)
	{
		free(actual_title);
		return false;
	}	

	free(actual_title);
	free(actual_artist);
	return true;
}

// Parse Input & abort if any strange input occurs
// better no lyrics instead accidentaly getting 'lady gaga' - 'pokerface'
memCache_t * lyrics_metrolyrics_parse(cb_object * capo)
{
    // Get sure that we're not on the 'nothing found page'
    if( strstr(capo->cache->data,"Found 0 Results in Everything") == NULL)
    {
        char *p_ref;

	// Search for first result TODO: Levenshtein
        if( (p_ref = strcasestr(capo->cache->data,"<ul id=\"results\">")) )
        {
            while( (p_ref = strcasestr(p_ref,"<a href=\"")) != NULL)
            {
		// extract path to page
                p_ref += strlen("<a href=\"");
                char * e_ref = strstr(p_ref,"\" title=");

		if(!e_ref) continue;

		// continue if valid
                if(e_ref)
                {
                    size_t b_len = e_ref - p_ref;
		    if(b_len > 1024)
		    {
			return NULL;
		    }

                    char * new_ref = malloc(b_len + 1);
                    strncpy(new_ref,p_ref,b_len);
                    new_ref[b_len] = '\0';

		    if( check_ref(new_ref, capo->artist, capo->title) == false) 
		    {
			if(p_ref == NULL)
			{
				return NULL;
			}
		    	p_ref = strcasestr(p_ref,"</li><li>"); 
			if(p_ref == NULL)
			{
				return NULL;
			}
			continue;
		    }

		    // construct real url
                    char * lyr_url = strdup_printf("http://www.metrolyrics.com%s.html",new_ref);
                    free(new_ref);

                    if(lyr_url)
                    {
			// download lyrics page
                        memCache_t * lyr_cache = download_single(lyr_url,1L);
                        free(lyr_url);

			// Check if DL is valid
                        if(lyr_cache && lyr_cache->data)
                        {
			    // Search for start of lyrics
                            char * l_ptr = strcasestr(lyr_cache->data, "<div id=\"lyrics\">");
                            if(l_ptr)
                            {
				// Search for end of lyrics
                                char * dive_ptr = strcasestr(l_ptr,"</div>");
                                if(dive_ptr)
                                {
                                    size_t copy_len =  dive_ptr - l_ptr;
                                    char * copy_buf = malloc(copy_len + 1);

                                    if(copy_buf && copy_len)
                                    {
					// Copy raw data
					// glyr takes care that all tags and unicode 
					// stuff gets replaced by right letters
                                        strncpy(copy_buf,l_ptr,copy_len);
                                        copy_buf[copy_len] = '\0';

					// Get read to return
                                        memCache_t * r_cache = DL_init();
                                        if(r_cache)
                                        {
					    // remove tags, because otherwise double newlines occur
                                            r_cache->data = strreplace(copy_buf,"<br />",NULL);
                                            r_cache->size = strlen(r_cache->data);
                                            free(copy_buf);

					    // Success!
                                            return r_cache;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Cascade action
    return NULL;
}
