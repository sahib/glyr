/***********************************************************
* This file is part of glyr
* + a commnadline tool and library to download various sort of musicrelated metadata.
* + Copyright (C) [2011]  [Christopher Pahl]
* + Hosted at: https://github.com/sahib/glyr
*
* glyr is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* glyr is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with glyr. If not, see <http://www.gnu.org/licenses/>.
**************************************************************/

#include <stdlib.h>
#include <string.h>

#include "allcdcovers.h"

#include "../core.h"
#include "../stringop.h"

const char * cover_allcdcovers_url(GlyQuery * sets)
{
    if(sets->cover.max_size >= 1000 && sets->cover.min_size != -1)
	return NULL;

    return "http://www.allcdcovers.com/search/music/all/%artist%+%album%";
}

#define NODE_BEGIN "/show/"
#define NODE_ENDIN "\">"

#define LINE_BEGIN "<div class=\"coverDetailsContainer\""
#define LINE_ENDIN "</div><div></td>"

#define DOWNLOAD_BEGIN "/download/"
#define DOWNLOAD_ENDIN "\">"

static GlyMemCache * get_cover_page(const char * url, GlyQuery * s)
{
	GlyMemCache * rt = NULL;
	GlyMemCache * dl = download_single(url,s,NULL);

	if(dl != NULL)
	{
		char * dl_link = strstr(dl->data,DOWNLOAD_BEGIN);
		if(dl_link != NULL)
		{
			char * cpy = copy_value(dl_link+strlen(DOWNLOAD_BEGIN),strstr(dl_link,DOWNLOAD_ENDIN));
			if(cpy != NULL)
			{
				rt = DL_init();
				rt->data = strdup_printf("http://www.allcdcovers.com/download/%s",cpy);
				rt->size = strlen(cpy);
				free(cpy);
			}
		}
		DL_free(dl);
	}
	return rt;
}

GlyCacheList * cover_allcdcovers_parse(cb_object * capo)
{
	GlyCacheList * r_list = NULL;
	char *  find = strstr(capo->cache->data,LINE_BEGIN);
	if(find != NULL)
	{
		char * terminate = strstr(find,LINE_ENDIN);
		while( (find = strstr(find+1,NODE_BEGIN)) && terminate > find)
		{
			char * get_end  = NULL;
			char * get_show = copy_value(find+strlen(NODE_BEGIN),(get_end = strstr(find,NODE_ENDIN)));
			if(get_show != NULL)
			{
				char * long_url = strdup_printf("http://www.allcdcovers.com/show/%s",get_show);
				GlyMemCache * mc = get_cover_page(long_url,capo->s);
				if(mc != NULL)
				{
					if(!r_list) r_list = DL_new_lst();
					fprintf(stderr,"U %s\n",mc->data);
					DL_add_to_list(r_list,mc);
				}
				free(get_show);
				free(long_url);

				find = get_end;
			}
			
		}
	}
	return r_list;
}
