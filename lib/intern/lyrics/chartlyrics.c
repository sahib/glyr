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

#define CL_API_URL "http://api.chartlyrics.com/apiv1.asmx/SearchLyric?artist=${artist}&song=${title}"
#define CL_API_GET "http://api.chartlyrics.com/apiv1.asmx/GetLyric?lyricId=%s&lyricCheckSum=%s"

/////////////////////////////////

static const gchar * lyrics_chartlyrics_url (GlyrQuery * s)
{
    return CL_API_URL;
}

/////////////////////////////////

#define LYRIC_TEXT_BEG "<Lyric>"
#define LYRIC_TEXT_END "</Lyric>"

static GlyrMemCache * get_lyrics_from_results (GlyrQuery * s, const gchar * url)
{
    GlyrMemCache * result = NULL;
    GlyrMemCache * dl_cache = download_single (url,s,NULL);
    if (dl_cache != NULL)
    {
        gchar * text = get_search_value (dl_cache->data,LYRIC_TEXT_BEG,LYRIC_TEXT_END);
        if (text != NULL)
        {
            result = DL_init();
            result->data = text;
            result->size = strlen (text);
            result->dsrc = g_strdup (url);
        }
        DL_free (dl_cache);
    }
    return result;
}

/////////////////////////////////

#define LYRIC_NODE "<SearchLyricResult>"

#define LYRIC_CHECKSUM_BEG "<LyricChecksum>"
#define LYRIC_CHECKSUM_END "</LyricChecksum>"

#define ARTIST_BEG "<Artist>"
#define ARTIST_END "</Artist>"

#define SONG_BEG "<Song>"
#define SONG_END "</Song>"

#define LYRIC_ID_BEG "<LyricId>"
#define LYRIC_ID_END "</LyricId>"

static GList * lyrics_chartlyrics_parse (cb_object * capo)
{
    GList * result_list = NULL;
    gchar * node = capo->cache->data;
    gint nodelen = (sizeof LYRIC_NODE) - 1;

    while (continue_search (g_list_length (result_list),capo->s) && (node = strstr (node + nodelen, LYRIC_NODE) ) != NULL)
    {
        node += nodelen;
        gchar * artist = get_search_value (node,ARTIST_BEG,ARTIST_END);
        gchar * title  = get_search_value (node,SONG_BEG,SONG_END);

        if (levenshtein_strnormcmp (capo->s,artist,capo->s->artist) <= capo->s->fuzzyness &&
                levenshtein_strnormcmp (capo->s,title,capo->s->title)   <= capo->s->fuzzyness)
        {
            gchar * lyric_id = get_search_value (node,LYRIC_ID_BEG,LYRIC_ID_END);
            gchar * lyric_checksum = get_search_value (node,LYRIC_CHECKSUM_BEG,LYRIC_CHECKSUM_END);
            if (lyric_id && lyric_checksum && strcmp (lyric_id,"0") != 0)
            {
                gchar * content_url = g_strdup_printf (CL_API_GET,lyric_id,lyric_checksum);
                GlyrMemCache * result = get_lyrics_from_results (capo->s,content_url);
                if (result != NULL)
                {
                    result_list = g_list_prepend (result_list,result);
                }
                g_free (content_url);
            }
            g_free (lyric_id);
            g_free (lyric_checksum);
        }
        g_free (artist);
        g_free (title);
    }
    return result_list;
}

/////////////////////////////////


MetaDataSource lyrics_chartlyrics_src =
{
    .name = "chartlyrics",
    .key  = 'c',
    .parser    = lyrics_chartlyrics_parse,
    .get_url   = lyrics_chartlyrics_url,
    .type      = GLYR_GET_LYRICS,
    .quality   = 75,
    .speed     = 25,
    .endmarker = NULL,
    .free_url  = false
};
