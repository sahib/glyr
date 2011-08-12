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
#include "../cover/amazon.h"
#include "../../core.h"
#include "../../stringlib.h"

const char * review_amazon_url(GlyQuery * settings)
{
    return generic_amazon_url(settings,"EditorialReview");
}

#define TheContent "<Content>"
#define TheEndofCt "</Content>"
GList * review_amazon_parse(cb_object * capo)
{
    char * node = capo->cache->data;
    size_t clen = strlen(TheContent);

    GList * cList = NULL;

    size_t iter = 0;
    while( (node = strstr(node+1,TheContent)) != NULL && continue_search(iter,capo->s))
    {
        char * endOfText = strstr(node+clen,TheEndofCt);
        char * text = copy_value(node+clen,endOfText);
        if(text)
        {
            // Ignore reviews with 300 chars
            // as mostly just advertisement
            if((endOfText - (node+clen)) > 350)
            {
                GlyMemCache * tmp = DL_init();
                tmp->data = beautify_lyrics(text);
                tmp->size = tmp->data ? strlen(tmp->data) : 0;
                tmp->dsrc = strdup(capo->url);

                iter++;

                cList = g_list_prepend(cList,tmp);
            }
            g_free(text);
        }
    }
    return cList;
}

/*--------------------------------------------------------*/

MetaDataSource review_amazon_src =
{
    .name = "amazon",
    .key  = 'a',
    .parser    = review_amazon_parse,
    .get_url   = review_amazon_url,
    .type      = GET_ALBUM_REVIEW,
    .quality   = 45,
    .speed     = 70,
    .endmarker = NULL,
    .free_url  = true
};
