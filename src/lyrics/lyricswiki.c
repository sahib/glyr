#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lyricswiki.h"

#include "../types.h"
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
bool lv_cmp_content(const char *to_artist, const char * to_title, cb_object * capo_p)
{
    size_t tmp_a_len = strstr(to_artist,"</artist>") -  to_artist;
    size_t tmp_t_len = strstr(to_artist,"</song>")   -  to_title;

    if(tmp_a_len < 1 || tmp_t_len < 1 || !to_artist || !to_title || !capo_p)
    {
        return false;
    }


    char * tmp_artist = malloc(tmp_a_len + 1);
    char * tmp_title  = malloc(tmp_t_len + 1);
    strncpy(tmp_artist, to_artist, tmp_a_len);
    strncpy(tmp_title,  to_title, tmp_t_len);

    tmp_artist[tmp_a_len] = '\0';
    tmp_title [tmp_t_len] = '\0';
    ascii_strdown_modify(tmp_artist,tmp_a_len);
    ascii_strdown_modify(tmp_title,tmp_t_len);

    char * cmp_a = ascii_strdown_modify(strdup_printf("<artist>%s",capo_p->artist),-1);
    char * cmp_t = ascii_strdown_modify(strdup_printf("<song>%s",  capo_p->title),-1);

    bool _r = true;

    if( ( levenshtein_strcmp(cmp_a,tmp_artist) + levenshtein_strcmp(cmp_t,tmp_title) ) > LV_MAX_DIST)
    {
        fprintf(stderr, "lyricswiki.c:warn(): levenshtein_strcmp() refused weird input");
        _r = false;
    }

    free(cmp_a);
    free(cmp_t);

    free(tmp_artist);
    free(tmp_title);

    return _r;
}


memCache_t * lyrics_lyricswiki_parse(cb_object * capo)
{
    char *find, *endTag;

    if(lv_cmp_content(strstr(capo->cache->data,"<artist>"),strstr(capo->cache->data,"<song>"),capo) == false)
    {
        return NULL;
    }

    if( (find = strstr(capo->cache->data,"<url>")) == NULL)
    {
        return NULL;
    }

    nextTag(find);

    while( (endTag = strstr(find, "</url>")) == NULL)
    {
        return NULL;
    }

    size_t len = ABS(endTag - find);
    char *wiki_page_url = malloc(len + 1);
    strncpy(wiki_page_url, find, len);
    wiki_page_url[len] = 0;

    if(capo->cache && capo->cache->size)
    {
        DL_free(capo->cache);
    }

    capo->cache = download_single(wiki_page_url, 1L);
    free(wiki_page_url);
    if(capo->cache != NULL)
    {
        char *lyr_begin, *lyr_end;
        if( (lyr_begin = strstr(capo->cache->data, "'17'/></a></div>")) == NULL)
        {
            return NULL;
        }

        nextTag(lyr_begin);
        nextTag(lyr_begin);
        nextTag(lyr_begin);

        if(lyr_begin == NULL)
        {
            return NULL;
        }

        if( (lyr_end = strstr(lyr_begin, "<!--")) == NULL)
        {
            return NULL;
        }

        size_t lyr_len = ABS(lyr_end - lyr_begin);
        char *lyr_utf8 = malloc(lyr_len + 1);
        strncpy(lyr_utf8, lyr_begin, lyr_len);

        char *result = unescape_html_UTF8(lyr_utf8);
        if(lyr_utf8) free(lyr_utf8);

        memCache_t * r_cache = DL_init();
        r_cache->data = result;
        r_cache->size = lyr_len;
        return r_cache;
    }

    return NULL;
}
