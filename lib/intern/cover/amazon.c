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
#include "../../glyr.h"

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
#define ACCESS_KEY API_KEY_AMAZON


#define rg_markup "__RESPONSE_GROUP__"
const char * generic_amazon_url(GlyQuery * sets, const char * response_group)
{
    const char * lang_link = NULL;
    if(sets->cover.min_size <= 500 || sets->cover.min_size)
    {
        if(!strcmp(sets->lang,"us"))
            lang_link =  "http://free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=%artist%+%album%\0";
        else if(!strcmp(sets->lang,"ca"))
            lang_link =  "http://ca.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=%artist%+%album%\0";
        else if(!strcmp(sets->lang,"uk"))
            lang_link =  "http://co.uk.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=%artist%+%album%\0";
        else if(!strcmp(sets->lang,"fr"))
            lang_link =  "http://fr.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=%artist%+%album%\0";
        else if(!strcmp(sets->lang,"de"))
            lang_link =  "http://de.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=%artist%+%album%\0";
        else if(!strcmp(sets->lang,"jp"))
            lang_link =  "http://co.jp.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=%artist%+%album%\0";
        else
            lang_link =  "http://free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup="rg_markup"&Keywords=%artist%+%album%\0";
    }
    if(lang_link != NULL)
    {
        return strreplace(lang_link,rg_markup,response_group);
    }
    return NULL;
}

const char * cover_amazon_url(GlyQuery * sets)
{
    return generic_amazon_url(sets,"Images");
}

#define C_MAX(X) (capo->s->cover.max_size <  X && capo->s->cover.max_size != -1)
#define C_MIN(X) (capo->s->cover.min_size >= X && capo->s->cover.min_size != -1)

GlyCacheList * cover_amazon_parse(cb_object *capo)
{
    const char *tag_ssize = (capo->s->cover.max_size == -1 && capo->s->cover.min_size == -1) ? "<LargeImage>"  :
                            (C_MAX( 30) && C_MIN(-1)) ? "<SwatchImage>" :
                            (C_MAX( 70) && C_MIN(30)) ? "<SmallImage>"  :
                            (C_MAX(150) && C_MIN(70)) ? "<MediumImage>" :
                            "<LargeImage>"  ;
#undef MAX
#undef MIN

    int urlc = 0;
    GlyCacheList * r_list = NULL;

    char * find = capo->cache->data;
    while( (find = strstr(find +1, tag_ssize)) != NULL && continue_search(urlc,capo->s))
    {
        /* Next two XML tags not relevant */
        nextTag(find);
        nextTag(find);
        char * endTag = NULL;
        if( (endTag = strstr(find, "</URL>")) != NULL)
        {
            char * result_url = copy_value(find,endTag);
            if(result_url)
            {
                if(!r_list) r_list = DL_new_lst();

                GlyMemCache * result_cache = DL_init();
                result_cache->data = result_url;
                result_cache->size = strlen(result_url);

                DL_add_to_list(r_list,result_cache);
                urlc++;
            }
        }
    }
    return r_list;
}

MetaDataSource cover_amazon_src = {
	.name      = "amazon",
	.key       = 'a',
	.parser    = cover_amazon_parse,
	.get_url   = cover_amazon_url,
	.type      = GET_COVERART,
	.endmarker = NULL,
	.free_url  = true
};
