/***********************************************************
* This file is part of glyr
* + a commnadline tool and library to download various sort of music related metadata.
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
#include "../common/amazon.h"

// Example snippet of what we parse:
/***

<SmallImage>
<URL>
http://ecx.images-amazon.com/images/I/51rnlRwtsiL._SL75_.jpg
</URL>
<Height Units="pixels">75</Height>
<Width Units="pixels">75</Width>
</SmallImage>

<MediumImage>
<URL>
http://ecx.images-amazon.com/images/I/51rnlRwtsiL._SL160_.jpg
</URL>
<Height Units="pixels">160</Height>
<Width Units="pixels">160</Width>
</MediumImage>
<LargeImage>
<URL>
http://ecx.images-amazon.com/images/I/51rnlRwtsiL.jpg
</URL>
<Height Units="pixels">455</Height>
<Width Units="pixels">455</Width>
</LargeImage>

***/

// A short note:
// Since some time amazon does not allow anonymous acces to their webservices.
// So you have to register an (free) account and you'll get an Accesskey and a Secretkey,
// sadly amazon does not allow you to publish the secretkey to the public,
// therefore I had to use freeapisign.com to access the webservices, the bad thing is:
// it is limited to 30K requests per month. If you have an own account: replace the Acceskey here,
// as an fallback there is the albumart and coverhunt plugin which implement a search on amazon (on serverside)
/////////////////////////////////

static const gchar *cover_amazon_url(GlyrQuery *sets)
{
    return generic_amazon_url(sets, "Images");
}

/////////////////////////////////

#define END_OF_URL "</URL>"
#define C_MAX(X) (capo->s->img_max_size <  X && capo->s->img_max_size != -1)
#define C_MIN(X) (capo->s->img_min_size >= X && capo->s->img_min_size != -1)

static GList *cover_amazon_parse(cb_object *capo)
{
    const gchar *tag_ssize = (capo->s->img_max_size == -1 && capo->s->img_min_size == -1) ? "<LargeImage>"  :
                             (C_MAX(30) && C_MIN(-1)) ? "<SwatchImage>" :
                             (C_MAX(70) && C_MIN(30)) ? "<SmallImage>"  :
                             (C_MAX(150) && C_MIN(70)) ? "<MediumImage>" :
                             "<LargeImage>"  ;

    GList *result_list = NULL;
    gchar *find = capo->cache->data;
    while(continue_search(g_list_length(result_list), capo->s) && (find = strstr(find + strlen(tag_ssize), tag_ssize)) != NULL) {
        /* Next two XML tags not relevant */
        nextTag(find);
        nextTag(find);

        gchar *endTag = NULL;
        if((endTag = strstr(find, END_OF_URL)) != NULL) {
            gchar *result_url = copy_value(find, endTag);
            if(result_url != NULL) {
                GlyrMemCache *result = DL_init();
                result->data = result_url;
                result->size = endTag - find;
                result_list = g_list_prepend(result_list, result);
            }
        }
    }
    return result_list;
}

/////////////////////////////////

MetaDataSource cover_amazon_src = {
    .name      = "amazon",
    .key       = 'a',
    .parser    = cover_amazon_parse,
    .get_url   = cover_amazon_url,
    .type      = GLYR_GET_COVERART,
    .quality   = 90,
    .speed     = 85,
    .endmarker = NULL,
    .free_url  = true,
    .lang_aware = true
};
