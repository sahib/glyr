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
#include "../../stringlib.h"
#include "../../core.h"
#include "../../glyr.h"

const char * ainfo_allmusic_url(GlyrQuery * s)
{
    return "http://www.allmusic.com/search/artist/%artist%";
}

#define IMG_BEGIN "<p class=\"text\">"
#define IMG_ENDIN "</p>"

GlyrMemCache * parse_bio_short(GlyrMemCache * to_parse)
{
    GlyrMemCache * rche = NULL;
    gchar * text_begin = strstr(to_parse->data,IMG_BEGIN);
    if(text_begin != NULL)
    {
        gchar * text_endin = strstr(text_begin,IMG_ENDIN);
        if(text_endin != NULL)
        {
            gchar * text = copy_value(text_begin + strlen(IMG_BEGIN),text_endin);
            if(text != NULL)
            {
                remove_tags_from_string(text,strlen(text),'<','>');
                gchar * null_marker = strstr(text,"&hellip;&nbsp;&nbsp;");
                if(null_marker != NULL)
                {
                    text[null_marker - text + 0] = '.';
                    text[null_marker - text + 1] = '.';
                    text[null_marker - text + 2] = '.';
                    text[null_marker - text + 3] =   0;
                }

                rche = DL_init();
                rche->data = strip_html_unicode(text);
                rche->size = strlen(rche->data);

                g_free(text);
                text = NULL;
            }
        }
    }
    return rche;
}

#define ROOT "<div id=\"tabs\">"
#define ROOT_URL "http://www.allmusic.com/artist/"

GlyrMemCache * parse_bio_long(GlyrMemCache * to_parse, GlyrQuery * s)
{
    GlyrMemCache * rche = NULL;
    char * root = strstr(to_parse->data,ROOT);
    if(root)
    {
        char * url_root = strstr(root,ROOT_URL);
        if(url_root != NULL)
        {
            char * id = copy_value(url_root + strlen(ROOT_URL),strstr(url_root,"\">"));
            if(id != NULL)
            {
                char * url = g_strdup_printf(ROOT_URL"%s",id);
                if(url != NULL)
                {
                    GlyrMemCache * dl = download_single(url,s,NULL);
                    if(dl != NULL)
                    {
                        rche = parse_bio_short(dl);
                        DL_free(dl);
                        dl=NULL;
                    }
                    g_free(url);
                    url=NULL;
                }
                g_free(id);
                id=NULL;
            }
        }
    }
    return rche;
}

// begin of search results
#define SEARCH_TREE_BEGIN "<table class=\"search-results\""

#define SEARCH_NODE "<td><a href=\""
#define SEARCH_DELM "\">"

GList * ainfo_allmusic_parse(cb_object * capo)
{
    GList * r_list = NULL;
    if( strstr(capo->cache->data, "<!--Begin Biography -->") )
    {
        GlyrMemCache * info_short = parse_bio_short(capo->cache);
        GlyrMemCache * info_long  = parse_bio_long (capo->cache,capo->s);

        r_list = g_list_prepend(r_list,info_short);
        r_list = g_list_prepend(r_list,info_long );
        return r_list;
    }

    char * search_begin = NULL;
    if( (search_begin = strstr(capo->cache->data, SEARCH_TREE_BEGIN)) == NULL)
    {
        return NULL;
    }

    int urlc = 0;
    char *  node = search_begin;
    while( (node = strstr(node+1,SEARCH_NODE)) && continue_search(urlc,capo->s))
    {
        char * url = copy_value(node+strlen(SEARCH_NODE),strstr(node,SEARCH_DELM));
        if(url != NULL)
        {
            GlyrMemCache * dl = download_single(url,capo->s,NULL);
            if(dl != NULL)
            {
                GlyrMemCache * info_short = parse_bio_short(dl);
                if(info_short != NULL)
                {
                    r_list = g_list_prepend(r_list,info_short);
                }

                GlyrMemCache * info_long  = parse_bio_long (dl,capo->s);
                if(info_long != NULL)
                {
                    r_list = g_list_prepend(r_list,info_long );
                }

                DL_free(dl);

                if(info_short || info_long)
                    urlc++;
            }
            g_free(url);
            url=NULL;
        }
    }
    return r_list;
}

/*-------------------------------------*/

MetaDataSource ainfo_allmusic_src =
{
    .name      = "allmusic",
    .key       = 'm',
    .free_url  = false,
    .type      = GET_ARTISTBIO,
    .parser    = ainfo_allmusic_parse,
    .get_url   = ainfo_allmusic_url,
    .quality   = 90,
    .speed     = 40,
    .endmarker = NULL
};
