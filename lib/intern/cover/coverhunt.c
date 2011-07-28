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

#include "../../stringlib.h"
#include "../../core.h"

/*
 * Provider will be removed if down any longer..
*/


const char * cover_coverhunt_url(GlyQuery * sets)
{
    if(sets->cover.min_size <= 500 || sets->cover.min_size == -1)
        return "http://www.coverhunt.com/index.php?query=%artist%+%album%&action=Find+my+CD+Covers";

    return NULL;
}

static bool check_size(const char * art_root, const char *hw, cb_object * capo)
{
    char * begin = strstr(art_root,hw);
    if(begin)
    {
        char * end = strchr(begin,' ');
        char * buf = copy_value(begin+strlen(hw),end);
        if(buf)
        {
            int atoid = atoi(buf);

            free(buf);
            if((atoid >= capo->s->cover.min_size || capo->s->cover.min_size == -1) &&
                    (atoid <= capo->s->cover.max_size || capo->s->cover.max_size == -1)  )
                return true;
        }
    }
    return false;
}

#define SEARCH_RESULT_BEGIN "<table><tr><td"
#define IMG_START "<img src=\""
#define NODE_BEGIN "<a href=\"/go/"

// Take the first link we find.
// coverhunt sadly offers  no way to check if the
// image is really related to the query we're searching for
GlyCacheList * cover_coverhunt_parse(cb_object *capo)
{
    GlyCacheList * r_list = NULL;

    // navigate to start of search results
    char * table_start;
    if( (table_start = strstr(capo->cache->data,SEARCH_RESULT_BEGIN)) == NULL)
    {
        return NULL;
    }

    int urlc = 0;


    while( (table_start = strstr(table_start + 1,NODE_BEGIN)) && continue_search(urlc,capo->s))
    {
        char * table_end = NULL;
        if( (table_end = strstr(table_start,"\">")) != NULL)
        {
            char * go_url = copy_value(table_start + strlen(NODE_BEGIN),table_end);
            if(go_url)
            {
                char * real_url = strdup_printf("http://www.coverhunt.com/go/%s",go_url);
                if(real_url)
                {
                    GlyMemCache * search_buf = download_single(real_url,capo->s,"<div id=\"right\">");
                    if(search_buf)
                    {
                        char * artwork = strstr(search_buf->data, "<div class=\"artwork\">");
                        if(artwork)
                        {
                            if(check_size(artwork,"height=",capo) && check_size(artwork,"width=",capo))
                            {
                                char * img_start = strstr(artwork,IMG_START);
                                if(img_start)
                                {
                                    img_start += strlen(IMG_START);
                                    char * img_end = strstr(img_start,"\" ");
                                    if(img_end)
                                    {
                                        char * url = copy_value(img_start,img_end);
                                        if(url)
                                        {
                                            if(!r_list) r_list = DL_new_lst();

                                            GlyMemCache * shell = DL_init();
                                            shell->data = url;
                                            shell->size = img_end - img_start;

                                            DL_add_to_list(r_list,shell);

                                            urlc++;
                                        }
                                    }
                                }
                            }
                        }
                        DL_free(search_buf);
                    }
                    free(real_url);
                }
                free(go_url);
            }
        }
    }
    return r_list;
}

MetaDataSource cover_coverhunt_src = {
	.name      = "coverhunt",
	.key       = 'c',
	.parser    = cover_coverhunt_parse,
	.get_url   = cover_coverhunt_url,
	.type      = GET_COVERART,
	.endmarker = "<div id=\"footer\">",
	.free_url  = false
};
