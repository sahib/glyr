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

#define SUMMARY_BEGIN "<summary><![CDATA["
#define SUMMARY_ENDIN "]]></summary>"

#define CONTENT_BEGIN "<content><![CDATA["
//#define CONTENT_ENDIN "]]></content>"
#define CONTENT_ENDIN "User-contributed text"

#define API_KEY API_KEY_LASTFM

const char * ainfo_lastfm_url(GlyrQuery * s)
{
    char * right_artist = strreplace(s->artist," ","+");
    char * url = g_strdup_printf("http://ws.audioscrobbler.com/2.0/?method=artist.getinfo&artist=%s&autocorrect=0&lang=%s&api_key="API_KEY,right_artist,s->lang);

    if(right_artist)
        g_free(right_artist);
    return url;
}

static char * fix_crappy_lastfm_txt(const char * txt)
{
    char * result = NULL;
    if(txt)
    {
        remove_tags_from_string((char*)txt,strlen(txt),'<','>');
        char * no_unicode = strip_html_unicode(txt);
        if(no_unicode)
        {
            char * no_unicode_at_all = unescape_html_UTF8(no_unicode);
            if(no_unicode_at_all)
            {
                char * trim_buf = g_malloc0(strlen(no_unicode_at_all)+1);
                if(trim_buf)
                {
                    trim_copy(no_unicode_at_all,trim_buf);
                    result = trim_buf;
                }
                g_free(no_unicode_at_all);
            }
            g_free(no_unicode);
        }
    }
    return result;
}

GList * ainfo_lastfm_parse(cb_object * capo)
{
    GList * r_lst = NULL;
    char * short_desc = copy_value(strstr(capo->cache->data,SUMMARY_BEGIN)+strlen(SUMMARY_BEGIN),strstr(capo->cache->data,SUMMARY_ENDIN));
    if(short_desc)
    {
        char * content = copy_value(strstr(capo->cache->data,CONTENT_BEGIN)+strlen(CONTENT_BEGIN),strstr(capo->cache->data,CONTENT_ENDIN));
        if(content)
        {
            GlyrMemCache * sc = DL_init();
            sc->data = fix_crappy_lastfm_txt(short_desc);
            sc->size = strlen(sc->data);
            sc->dsrc = strdup(capo->url);

            GlyrMemCache * lc = DL_init();
            lc->data = fix_crappy_lastfm_txt(content);
            lc->size = strlen(lc->data);
            lc->dsrc = strdup(capo->url);

            r_lst = g_list_prepend(r_lst,sc);
            r_lst = g_list_prepend(r_lst,lc);

            g_free(content);
            content=NULL;
        }
        g_free(short_desc);
        short_desc=NULL;
    }
    return r_lst;
}

/*-------------------------------------*/

MetaDataSource ainfo_lastfm_src =
{
    .name      = "lastfm",
    .key       = 'l',
    .free_url  = true,
    .type      = GET_ARTISTBIO,
    .parser    = ainfo_lastfm_parse,
    .get_url   = ainfo_lastfm_url,
    .quality   = 85,
    .speed     = 85,
    .endmarker = NULL
};
