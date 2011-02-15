#include <stdlib.h>
#include <string.h>

#include "amazon.h"

#include "../core.h"
#include "../stringop.h"
#include "../core.h"
#include "../glyr.h"

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


/***

Country settings:

0) US
1) Canada
2) UK
3) France
4) Germany
5) Japan

***/

// A short note:
// Since some time amazon does not allow anonymous acces to their webservices.
// So you have to register an (free) account and you'll get an Accesskey and a Secretkey,
// sadly amazon does not allow you to publish the secretkey to the public,
// therefore I had to use freeapisign.com to access the webservices, the bad thing is:
// it is limited to 30K requests per month. If you have an own account: replace the Acceskey here,
// as an fallback there is the albumart and coverhunt plugin which implement a search on amazon (on serverside)
#define ACCESS_KEY "AKIAJ6NEA642OU3FM24Q"

const char * cover_amazon_url(glyr_settings_t * sets)
{
    if(sets->cover.min_size <= 500 || sets->cover.min_size)
    {
        switch(sets->AMAZON_LANG_ID)
        {
        case  GLYR_AMAZON_US:
            return "http://free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup=Images&Keywords=%artist%+%album%\0";
        case  GLYR_AMAZON_CA:
            return "http://ca.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup=Images&Keywords=%artist%+%album%\0";
        case  GLYR_AMAZON_UK:
            return "http://co.uk.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup=Images&Keywords=%artist%+%album%\0";
        case  GLYR_AMAZON_FR:
            return "http://fr.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup=Images&Keywords=%artist%+%album%\0";
        case  GLYR_AMAZON_DE:
            return "http://de.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup=Images&Keywords=%artist%+%album%\0";
        case  GLYR_AMAZON_JP:
            return "http://co.jp.free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup=Images&Keywords=%artist%+%album%\0";
        default:
            return "http://free.apisigning.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="ACCESS_KEY"&Operation=ItemSearch&SearchIndex=Music&ResponseGroup=Images&Keywords=%artist%+%album%\0";
        }
    }
    return NULL;
}

#define MAX(X) (capo->s->cover.max_size <  X && capo->s->cover.max_size != -1)
#define MIN(X) (capo->s->cover.min_size >= X && capo->s->cover.min_size != -1)

memCache_t * cover_amazon_parse(cb_object *capo)
{
    char * find, * endTag;
    char *tag_ssize = (capo->s->cover.max_size == -1 && capo->s->cover.min_size == -1) ? "<LargeImage>"  :
                      (MAX( 30) && MIN(-1)) ? "<SwatchImage>" :
                      (MAX( 70) && MIN(30)) ? "<SmallImage>"  :
                      (MAX(150) && MIN(70)) ? "<MediumImage>" :
                      "<LargeImage>"  ;

#undef MAX
#undef MIN

    if( (find = strstr(capo->cache->data, tag_ssize)) == NULL)
    {
        return NULL;
    }

    /* Next two XML tags not relevant */
    nextTag(find);
    nextTag(find);

    if( (endTag = strstr(find, "</URL>")) == NULL)
    {
        return NULL;
    }

    char * result_url = copy_value(find,endTag);

    memCache_t *result_cache = DL_init();
    result_cache->data = result_url;
    result_cache->size = strlen(result_url);
    return result_cache;
}

