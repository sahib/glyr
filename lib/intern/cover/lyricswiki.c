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

const gchar * cover_lyricswiki_url (GlyrQuery * sets)
{
    const gchar * url = NULL;
    if (sets->img_min_size < 650)
    {
        url = "http://lyrics.wikia.com/api.php?format=xml&action=query&list=allimages&aiprefix=${artist}";
    }
    return url;
}

/////////////////////////////////

static gboolean check_file_format (GlyrQuery * query, gchar * filename)
{
    gboolean result = FALSE;
    gsize length = strlen (filename);
    gsize offset = 0;
    gchar *token = NULL;

    gchar ** token_list = g_strsplit (query->allowed_formats,GLYR_DEFAULT_FROM_ARGUMENT_DELIM,0);
    while ( (token = token_list[offset]) != NULL)
    {
        if (g_str_has_suffix (filename,token) == TRUE)
        {
            result = TRUE;
            gsize format_pos = length - strlen (token);
            filename[format_pos] = '\0';
            if (format_pos != 0 && filename[format_pos-1] == '.')
            {
                filename[format_pos-1] = '\0';
            }
            break;
        }
        offset++;
    }
    g_strfreev (token_list);
    return result;
}

/**
  -- Example snippet
  <img name="Axxis_-_Access_All_Areas.jpg"
  timestamp="2010-08-03T17:05:20Z"
  url="http://images.wikia.com/lyricwiki/images/f/f9/Axxis_-_Access_All_Areas.jpg"
  descriptionurl="http://lyrics.wikia.com/File:Axxis_-_Access_All_Areas.jpg"
  />
 **/

#define IMG_TAG "_-_"
#define END_TAG "\" timestamp=\""
#define URL_MARKER "url=\""
#define URL_END "\" descriptionurl="
#define NEXT_NAME "<img name=\""

GList * cover_lyricswiki_parse (cb_object * capo)
{
    gchar * find = capo->cache->data;
    gchar * endTag   = NULL;
    GList * result_list = NULL;

    gchar * escaped_album_name = strreplace (capo->s->album," ","_");
    if (escaped_album_name != NULL)
    {
        /* Go through all names and compare them with Levenshtein */
        while (continue_search (g_list_length (result_list),capo->s) && (find = strstr (find+ (sizeof IMG_TAG) - 1,IMG_TAG) ) != NULL)
        {
            /* Find end & start of the name */
            find  += (sizeof IMG_TAG) - 1;
            endTag = strstr (find,END_TAG);

            if (endTag == NULL || endTag <= find)
                continue;

            /* Copy the name of the current album */
            gchar * name = copy_value (find,endTag);
            if (name != NULL)
            {
                if (check_file_format (capo->s,name) && levenshtein_strnormcmp (capo->s,escaped_album_name,name) <= capo->s->fuzzyness)
                {
                    gchar * url = get_search_value (endTag, URL_MARKER, URL_END);
                    if (url != NULL)
                    {
                        GlyrMemCache * result = DL_init();
                        result->data = url;
                        result->size = strlen (url);
                        result_list = g_list_prepend (result_list,result);
                    }
                }

                /* Get next img tag */
                find = strstr (endTag,NEXT_NAME);
                g_free (name);

                /* Whoops, right into nonexistence.. */
                if (find == NULL) break;
            }
        }
        g_free (escaped_album_name);
    }
    return result_list;
}

/////////////////////////////////

MetaDataSource cover_lyricswiki_src =
{
    .name      = "lyricswiki",
    .key       = 'w',
    .parser    = cover_lyricswiki_parse,
    .get_url   = cover_lyricswiki_url,
    .type      = GLYR_GET_COVERART,
    .quality   = 75,
    .speed     = 85,
    .endmarker = NULL,
    .free_url  = false
};
