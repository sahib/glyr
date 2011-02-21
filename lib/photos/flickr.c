#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "flickr.h"

#include "../core.h"
#include "../core.h"
#include "../stringop.h"

#define LINE_BEGIN "<photo id="
#define LINE_ENDIN "/>"

const char * photos_flickr_url(glyr_settings_t * settings)
{
    return strdup_printf("http://api.flickr.com/services/rest/"
                         "?method=flickr.photos.search&"
                         "api_key=b5af0c3230fb478d53b20835223d57a4&"
                         "tags=%s&"
                         "content_type=1&"
                         "media=photos&"
                         "is_gallery=true&"
                         "per_page=%d",
                         settings->artist,
                         settings->number
                        );
}

static char * get_field_by_name(const char * string, const char * name)
{
    if(string)
    {
        char * find = strstr(string,name);
        if(find)
        {
            find += strlen(name);

            if(*find == '"')
                find ++;

            char * end = strstr(find,"\"");
            if(end)
            {
                return copy_value(find,end);
            }
        }
    }
    return NULL;
}

cache_list * photos_flickr_parse(cb_object * capo)
{
    // Needed: ID,secret,server,farm
    char * ph_begin = capo->cache->data;
    size_t urlc = 0;
    cache_list * r_list = NULL;

    while( (ph_begin=strstr(ph_begin,LINE_BEGIN)) != NULL && urlc < capo->s->number && urlc < capo->s->plugmax)
    {
        if(! *(++ph_begin))
            continue;

        char * ph_end = strstr(ph_begin,LINE_ENDIN);
        if(ph_end)
        {
            char * linebf = copy_value(ph_begin,ph_end);
            if(linebf)
            {
                char * ID = get_field_by_name(linebf, "id=");
                char * SC = get_field_by_name(linebf, "secret=");
                char * SV = get_field_by_name(linebf, "server=");
                char * FR = get_field_by_name(linebf, "farm=");
                free(linebf);
                linebf = NULL;

                if(!r_list) r_list = DL_new_lst();
                memCache_t * cache = DL_init();
                cache->data = strdup_printf("http://farm%s.static.flickr.com/%s/%s_%s.jpg",FR,SV,ID,SC);
                DL_add_to_list(r_list,cache);

                if(ID)
                    free(ID);
                if(SC)
                    free(SC);
                if(SV)
                    free(SV);
                if(FR)
                    free(FR);

                urlc++;
            }
        }
    }
    return r_list;
}
