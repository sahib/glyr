#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>

#include "google.h"

#include "../core.h"
#include "../stringop.h"

#define FIRST_RESULT "<td align=left valign=bottom width=23% style=\"padding-top:1px\"><a href=/imgres?imgurl="
#define END_OF_URL   "&imgrefurl="

const char * cover_google_url(GlyQuery * sets)
{
    const char * lang = NULL;
    if(!strcasecmp(sets->lang,"us"))
        lang = "com"  ;
    else if(!strcasecmp(sets->lang,"ca"))
        lang = "ca"   ;
    else if(!strcasecmp(sets->lang,"uk"))
        lang = "co.uk";
    else if(!strcasecmp(sets->lang,"fr"))
        lang = "fr"   ;
    else if(!strcasecmp(sets->lang,"de"))
        lang = "de"   ;
    else if(!strcasecmp(sets->lang,"jp"))
        lang = "co.jp";
    else
        lang = "com";

    const char * back = NULL;

    // Only use the 'large' option if really set high (>1000), large is usually verrrry incorrrect.
    if(sets->cover.min_size == -1)
    {
        back = "islt;qsvga";
    }
    else if(sets->cover.min_size <  50 && sets->cover.max_size <= 150)
    {
        back = "isz:i";
    }
    else if(sets->cover.min_size < 150 && sets->cover.max_size <= 300)
    {
        back = "isz:s";
    }
    else if(sets->cover.min_size < 300 && sets->cover.max_size <= 1000)
    {
        back = "islt:qsvga";
    }
    else
    {
        back = "islt:vga";
    }

    return strdup_printf("http://www.google.%s/images?q=%s+%s+album&safe=off&tbs=isch:1,iar:s,%s",lang,sets->artist,sets->album,back);
}

GlyCacheList * cover_google_parse(cb_object * capo)
{
    // we blindly take the first result
    // actually we have not much of a choice (no, not Nr.42)
    GlyCacheList * r_list = NULL;

    size_t urlc = 0;
    char * find = capo->cache->data;
    while( (find =  strstr(find+1,FIRST_RESULT)) != NULL && continue_search(urlc,capo->s))
    {
        char * end_of_url;
        find += strlen(FIRST_RESULT);
        if((end_of_url = strstr(find, END_OF_URL)) != NULL)
        {
            char * url = copy_value(find,end_of_url);
            if(url)
            {
                if(!r_list) r_list = DL_new_lst();

                GlyMemCache * result = DL_init();
                result->data = url;
                result->size = strlen(url);

                DL_add_to_list(r_list,result);

                urlc++;
            }
        }
    }
    return r_list;
}
