#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lastfm.h"

#include "../core.h"
#include "../stringop.h"

#define SUMMARY_BEGIN "<summary><![CDATA["
#define SUMMARY_ENDIN "]]></summary>"

#define CONTENT_BEGIN "<content><![CDATA["
//#define CONTENT_ENDIN "]]></content>"
#define CONTENT_ENDIN "User-contributed text"

#define API_KEY "7199021d9c8fbae507bf77d0a88533d7"

const char * ainfo_lastfm_url(glyr_settings_t * s)
{
    const char * lang = NULL;
    switch(s->lang)
    {
    case  GLYRL_US:
        lang = "en"  ;
        break;
    case  GLYRL_CA:
        lang = "en"  ;
        break;
    case  GLYRL_UK:
        lang = "en"  ;
        break;
    case  GLYRL_FR:
        lang = "fr"  ;
        break;
    case  GLYRL_DE:
        lang = "de"  ;
        break;
    case  GLYRL_JP:
        lang = "ja"  ;
        break;
    default:
        lang = "en";
    }
    char * right_artist = strreplace(s->artist," ","+");
    char * url = strdup_printf("http://ws.audioscrobbler.com/2.0/?method=artist.getinfo&artist=%s&autocorrect=0&lang=%s&api_key="API_KEY,right_artist,lang);

    if(right_artist)
        free(right_artist);
    return url;
}

static char * fix_crappy_lastfm_txt(const char * txt)
{
    char * result = NULL;
    if(txt)
    {
        char * no_html = remove_html_tags_from_string(txt,strlen(txt));
        if(no_html)
        {
            char * no_unicode = strip_html_unicode(no_html);
            if(no_unicode)
            {
                char * no_unicode_at_all = unescape_html_UTF8(no_unicode);
                if(no_unicode_at_all)
                {
                    char * trim_buf = calloc(strlen(no_unicode_at_all)+1,sizeof(char));
                    if(trim_buf)
                    {
                        trim_copy(no_unicode_at_all,trim_buf);
                        result = trim_buf;
                    }
                    free(no_unicode_at_all);
                }
                free(no_unicode);
            }
            free(no_html);
        }
    }
    return result;
}

cache_list * ainfo_lastfm_parse(cb_object * capo)
{
    cache_list * r_lst = NULL;
    char * short_desc = copy_value(strstr(capo->cache->data,SUMMARY_BEGIN)+strlen(SUMMARY_BEGIN),strstr(capo->cache->data,SUMMARY_ENDIN));
    if(short_desc)
    {
        char * content = copy_value(strstr(capo->cache->data,CONTENT_BEGIN)+strlen(CONTENT_BEGIN),strstr(capo->cache->data,CONTENT_ENDIN));
        if(content)
        {
            memCache_t * sc = DL_init();
            sc->data = fix_crappy_lastfm_txt(short_desc);
            sc->size = strlen(sc->data);
            sc->dsrc = strdup(capo->url);

            memCache_t * lc = DL_init();
            lc->data = fix_crappy_lastfm_txt(content);
            lc->size = strlen(lc->data);
            lc->dsrc = strdup(capo->url);

            r_lst = DL_new_lst();
            DL_add_to_list(r_lst,sc);
            DL_add_to_list(r_lst,lc);

            free(content);
            content=NULL;
        }
        free(short_desc);
        short_desc=NULL;
    }
    return r_lst;
}
