#include <stdlib.h>
#include <string.h>

#include "directlyrics.h"

#include "../types.h"
#include "../core.h"
#include "../stringop.h"

// directlyrics only offers google/javascipt powered search
// Unable to use this via CGI without lots of pain..
// So only use dump URL scheme

const char * lyrics_directlyrics_url(glyr_settings_t * settings)
{
    char * esc_a = ascii_strdown(settings->artist,-1);
    char * esc_t = ascii_strdown(settings->title, -1);

    if(esc_a && esc_t)
    {
        char * rep_a = strreplace(esc_a," ","-");
        char * rep_t = strreplace(esc_t," ","-");

        free(esc_a);
        free(esc_t);

        if(rep_a && rep_t)
        {
            char * url = strdup_printf("http://www.directlyrics.com/%s-%s-lyrics.html",rep_a,rep_t);
            free(rep_a);
            free(rep_t);
            return url;
        }
    }
    return NULL;
}

memCache_t * lyrics_directlyrics_parse(cb_object * capo)
{
    char * f_entry;
    char * f_end;

    if( (f_entry = strstr(capo->cache->data,"<div id=\"lyricsContent\"><p>")) == NULL)
    {
        return NULL;
    }

    f_entry += strlen("<div id=\"lyricsContent\"><p>");
    if( (f_end = strstr(f_entry,"</div>")) == NULL)
    {
        return NULL;
    }

    size_t len = f_end - f_entry;
    char * buf = malloc(len+1);
    if(buf)
    {
        strncpy(buf,f_entry,len);
        buf[len] = '\0';

        // replace nonsense brs
        char * brtagged = strreplace(buf,"<br>","");
        free(buf);
        if(brtagged)
        {
            memCache_t * result = DL_init();
            result->data = brtagged;
            result->size = strlen(brtagged);
            return result;
        }
    }
    return NULL;
}


