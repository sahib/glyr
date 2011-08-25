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

#include "../../core.h"
#include "../../stringlib.h"

const gchar * cover_albumart_url(GlyrQuery * sets)
{
    gint i = sets->img_min_size;
    gint e = sets->img_max_size;

    if((e >= 50 || e==-1) && (i == -1 || i < 450))
	{
        return "http://www.albumart.org/index.php?srchkey=%artist%+%album%&itempage=1&newsearch=1&searchindex=Music";
	}
    return NULL;
}

#define AMZ "http://ecx.images-amazon.com/images/"
#define NODE_START "<div id=\"main\">"
#define NODE_NEXT

GList * cover_albumart_parse(cb_object * capo)
{
    GList * result_list = NULL;
    gchar * node = strstr(capo->cache->data,NODE_START);

    if(node != NULL)
    {
        gsize size_it = 2;
        if(capo->s->img_max_size < 450 && capo->s->img_max_size != -1 && capo->s->img_min_size < 160)
        {
            size_it = 1;
        }

        while(continue_search(urlc,capo->s) (node = strstr(node+1,"<li><div style=\"")))
        {
            size_t i = 0;
            char * img_tag = node;
            char * img_end = NULL;

            for(i = 0; i < size_it; i++, img_tag += strlen(AMZ))
                if((img_tag = strstr(img_tag,AMZ)) == NULL)
                    break;

            if( (img_end  = strstr(img_tag,".jpg")) != NULL)
            {
                char * img_url = copy_value(img_tag,img_end);
                if(img_url)
                {
                    GlyrMemCache * result = DL_init();
                    result->data = g_strdup_printf(AMZ"%s.jpg",img_url);
                    result->size = strlen(result->data);

                    r_list = g_list_prepend(r_list,result);
                    urlc++;

                    g_free(img_url);
                }
            }
        }
    }
    return r_list;
}

MetaDataSource cover_albumart_src =
{
    .name      = "albumart",
    .key       = 'b',
    .parser    = cover_albumart_parse,
    .get_url   = cover_albumart_url,
    .type      = GET_COVERART,
    .endmarker = "<div id=\"pagination\"",
    .quality   = 80,
    .speed     = 70,
    .free_url  = false
};
