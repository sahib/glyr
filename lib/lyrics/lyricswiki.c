#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lyricswiki.h"

#include "../core.h"
#include "../core.h"
#include "../stringop.h"

#define LW_URL "http://lyrics.wikia.com/api.php?action=lyrics&fmt=xml&func=getSong&artist=%artist%&song=%title%"

const char * lyrics_lyricswiki_url(glyr_settings_t * settings)
{
    return LW_URL;
}

#define LV_MAX_DIST 8

// Compare response, so lyricswiki's search did not fool us
// This is to prevent completely wrong results, therfore the quite high tolerance
bool lv_cmp_content(const char *to_artist, const char * to_title, cb_object * capo)
{
    bool res = false;
    if(to_artist && to_title && capo)
    {
        char * tmp_artist = copy_value(to_artist,strstr(to_artist,"</artist>"));
        if(tmp_artist)
        {
            ascii_strdown_modify(tmp_artist,-1);
            char * tmp_title  = copy_value(to_title, strstr(to_title ,"</song>" ));
            if(tmp_title)
            {
                ascii_strdown_modify(tmp_title,-1);
                char * cmp_a = ascii_strdown_modify(strdup_printf("<artist>%s",capo->s->artist),-1);
                if(cmp_a)
                {
                    char * cmp_t = ascii_strdown_modify(strdup_printf("<song>%s",  capo->s->title),-1);
                    if(cmp_t)
                    {
                        if( ( levenshtein_strcmp(cmp_a,tmp_artist) + levenshtein_strcmp(cmp_t,tmp_title) ) <= LV_MAX_DIST)
                        {
                            res = true;
                        }
                        free(cmp_t);
                    }
                    free(cmp_a);
                }
                free(tmp_title);
            }
            free(tmp_artist);
        }
    }
    return res;
}


cache_list * lyrics_lyricswiki_parse(cb_object * capo)
{
    memCache_t * result = NULL;
    cache_list * r_list = NULL;
    if(lv_cmp_content(strstr(capo->cache->data,"<artist>"),strstr(capo->cache->data,"<song>"),capo))
    {
        char *find, *endTag;
        if( (find = strstr(capo->cache->data,"<url>")))
        {
            nextTag(find);
            if( (endTag = strstr(find, "</url>")))
            {
                char * wiki_page_url = copy_value(find,endTag);
                if(wiki_page_url)
                {
                    memCache_t * new_cache = download_single(wiki_page_url, capo->s);
                    if(new_cache)
                    {
                        char *lyr_begin, *lyr_end;
                        if( (lyr_begin = strstr(new_cache->data, "'17'/></a></div>")) )
                        {
                            nextTag(lyr_begin);
                            nextTag(lyr_begin);
                            nextTag(lyr_begin);

                            if( (lyr_end = strstr(lyr_begin, "<!--")) )
                            {
                                char * lyr = copy_value(lyr_begin,lyr_end);
                                if(lyr)
                                {
                                    result = DL_init();
                                    result->data = lyr;
                                    result->size = ABS(lyr_end - lyr_begin);
                                    result->dsrc = strdup(wiki_page_url);
                                }
                                lyr_end=NULL;
                            }
                            lyr_begin=NULL;
                        }
                        DL_free(new_cache);
                    }
                    free(wiki_page_url);
                }
                endTag=NULL;
            }
            find=NULL;
        }
    }
    if(result)
    {
        r_list = DL_new_lst();
        DL_add_to_list(r_list,result);
    }
    return r_list;
}
