/***********************************************************
* This file is part of glyr
* + a commnadline tool and library to download various sort of music related metadata.
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

/* Simple URL replace scheme - works pretty good here */
#define ELYRICS_BASE_URL "http://www.elyrics.net/read/%c/%s-lyrics/%s-lyrics.html"

static const gchar * lyrics_elyrics_url (GlyrQuery * settings)
{
    gchar * result_url = NULL;

    gchar * space_to_min_artist = strreplace (settings->artist," ","-");
    gchar * space_to_min_title  = strreplace (settings->title, " ","-");
    if (space_to_min_artist && space_to_min_title)
    {
        gchar * prep_title = NULL, * prep_artist = NULL;
        if (settings->normalization & GLYR_NORMALIZE_ARTIST)
            prep_title  = prepare_string (space_to_min_title, settings->normalization,TRUE);
        else
            prep_title  = prepare_string (space_to_min_title, GLYR_NORMALIZE_NONE,TRUE);

        if (settings->normalization & GLYR_NORMALIZE_ARTIST)
            prep_artist = prepare_string (space_to_min_artist, settings->normalization,TRUE);
        else
            prep_artist = prepare_string (space_to_min_artist, GLYR_NORMALIZE_NONE,TRUE);

        if (prep_title && prep_artist)
        {
            result_url = g_strdup_printf (ELYRICS_BASE_URL,prep_artist[0],prep_artist,prep_title);
            g_free (prep_title);
            g_free (prep_artist);
        }

        g_free (space_to_min_artist);
        g_free (space_to_min_title);
    }
    return result_url;
}

/////////////////////////////////

/* Neat try elyrics to vary this string with every request :-) */
#define FROM_MIDDLE "http://www.elyrics.net"
#define FROM_END   "<br><br>"
#define BAD_STRING "Lyrics removed for copyright protection!"

/* This data is separated from the actual lyrics => remove it from here */
static void remove_from_from_string (gchar * string)
{
    gchar * from_middle = strstr (string,FROM_MIDDLE);
    if (from_middle != NULL)
    {
        gchar * from_end = strstr (from_middle,FROM_END);
        if (from_end != NULL)
        {
            gchar * from_start = from_middle;
            while (from_start[0] && from_start[0] != '>')
            {
                from_start--;
            }

            if (from_start != NULL)
            {
                gsize memlen = from_end - from_start;
                memset (from_start,' ',memlen);
            }
        }
    }
}

/////////////////////////////////

#define LYRICS_BEGIN "<div class='ly' style='font-size:12px;'>"
#define LYRICS_ALT_END "these lyrics are submitted by"
#define LYRICS_END   "</div>"

static GList * lyrics_elyrics_parse (cb_object * capo)
{
    GList * results = NULL;
    gchar * lyrics_begin = strstr (capo->cache->data,LYRICS_BEGIN);
    if (lyrics_begin != NULL)
    {
        if (g_strstr_len (lyrics_begin,250,BAD_STRING) == NULL)
        {
            gchar * lyrics_end = strstr (lyrics_begin,LYRICS_ALT_END);
            if (lyrics_end == NULL)
            {
                lyrics_end = strstr (lyrics_begin,LYRICS_END);
            }

            if (lyrics_end != NULL)
            {
                /* Modifying original buffer is allowed
                 * As long it's not saved in the result cache
                 * */
                lyrics_end[0] = '\0';

                GlyrMemCache * item = DL_init();
                remove_from_from_string (lyrics_begin);
                item->data = g_strdup (lyrics_begin);
                item->size = lyrics_end - lyrics_begin;
                results = g_list_prepend (results,item);
            }
        }
    }
    return results;
}

/////////////////////////////////

MetaDataSource lyrics_elyrics_src =
{
    .name = "elyrics",
    .key  = 'e',
    .encoding  = "LATIN1",
    .parser    = lyrics_elyrics_parse,
    .get_url   = lyrics_elyrics_url,
    .type      = GLYR_GET_LYRICS,
    .endmarker = NULL,
    .quality   = 75,
    .speed     = 75,
    .free_url  = true
};
