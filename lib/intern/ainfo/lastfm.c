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

#define CONTENT_BEGIN "<content><![CDATA["
#define CONTENT_ENDIN "User-contributed text"
#define OTHER_ENDIN "]]></content>"

/* Locales that are just mapped to 'en' */
const char * locale_map_to_en = "ca|uk|us";

static const gchar * ainfo_lastfm_url (GlyrQuery * s)
{
    gchar * url = NULL;
    gchar * right_artist = strreplace (s->artist," ","+");
    if (right_artist != NULL)
    {
        gchar * lang = "en";

        /* Check if this is an allowed language */
        if (strstr (GLYR_DEFAULT_SUPPORTED_LANGS,s->lang) != NULL)
        {
            lang = (gchar*) s->lang;
        }

        /* Do we need to map a language to 'en'? */
        if (strstr (locale_map_to_en,s->lang) != NULL)
        {
            lang = "en";
        }

        url = g_strdup_printf ("http://ws.audioscrobbler.com/2.0/?method=artist.getinfo&autocorrect=1&artist=%s&lang=%s&api_key="API_KEY_LASTFM,right_artist,lang);
        g_free (right_artist);
    }
    return url;
}

/////////////////////////////////

static GList * ainfo_lastfm_parse (cb_object * capo)
{
    GList * result_list = NULL;
    gchar * content_begin = strstr (capo->cache->data,CONTENT_BEGIN);
    gchar * content_endin = strstr (capo->cache->data,CONTENT_ENDIN);
    if (content_endin == NULL)
    {
        content_endin = strstr (capo->cache->data,OTHER_ENDIN);
    }

    if (content_begin && content_endin)
    {
        content_begin += (sizeof CONTENT_BEGIN) - 1;
        gchar * content = copy_value (content_begin,content_endin);
        if (content != NULL)
        {
            GlyrMemCache * result = DL_init();
            result->data = content;
            result->size = strlen (result->data);
            result_list = g_list_prepend (result_list,result);
        }
    }
    return result_list;
}

/////////////////////////////////

MetaDataSource ainfo_lastfm_src =
{
    .name      = "lastfm",
    .key       = 'l',
    .free_url  = true,
    .type      = GLYR_GET_ARTIST_BIO,
    .parser    = ainfo_lastfm_parse,
    .get_url   = ainfo_lastfm_url,
    .quality   = 95,
    .speed     = 85,
    .endmarker = NULL,
    .lang_aware = true
};
