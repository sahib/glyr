/***********************************************************
* This file is part of glyr
* + a commnadline tool and library to download various sort of musicrelated metadata.
* + Copyright (C) [2011] [Christopher Pahl]
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

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "core.h"
#include "types.h"
#include "stringlib.h"

#include "lyrics/lyricswiki.h"
#include "lyrics/magistrix.h"
#include "lyrics/lyrix_at.h"
#include "lyrics/darklyrics.h"
#include "lyrics/lyricsvip.h"
#include "lyrics/directlyrics.h"
#include "lyrics/songlyrics.h"
#include "lyrics/lyrdb.h"
#include "lyrics/metrolyrics.h"
#include "lyrics/lyricstime.h"
#include "lyrics/lyricsreg.h"

#define LATE "<div id='d_navigation'"
#define DIRE "<!-- google_ad_section_start(weight=ignore) -->"
#define VIPE "</td></tr></table>"
#define DARE "<div class=\"note\">"

// Add your's here
GlyPlugin lyric_providers[] =
{
// full name key coloredname use? parser callback geturl callback free url?
    {"lyricswiki",  "w", "lyricswiki",  false, {lyrics_lyricswiki_parse,   lyrics_lyricswiki_url,   NULL, false}, GRP_SAFE | GRP_FAST},
    {"lyr.db",      "d", "lyr.db",      false, {lyrics_lyrdb_parse,        lyrics_lyrdb_url,        NULL, false}, GRP_SAFE | GRP_FAST},
    {"lyrix.at",    "a", "lyrix.at",    false, {lyrics_lyrixat_parse,      lyrics_lyrixat_url,      LATE, false}, GRP_SAFE | GRP_SLOW},
    {"magistrix",   "x", "magistrix",   false, {lyrics_magistrix_parse,    lyrics_magistrix_url,    NULL, false}, GRP_USFE | GRP_FAST},
    {"lyricstime",  "t", "lyricstime",  false, {lyrics_lyricstime_parse,   lyrics_lyricstime_url,   NULL, false}, GRP_SAFE | GRP_FAST},
    {"lyricsreg",   "r", "lyricsreg",   false, {lyrics_lyricsreg_parse,    lyrics_lyricsreg_url,    NULL, false}, GRP_USFE | GRP_FAST},
    {"directlyrics","i", "directlyrics",false, {lyrics_directlyrics_parse, lyrics_directlyrics_url, DIRE, true }, GRP_SAFE | GRP_FAST},
    {"lyricsvip",   "v", "lyricsvip",   false, {lyrics_lyricsvip_parse,    lyrics_lyricsvip_url,    VIPE, true }, GRP_USFE | GRP_FAST},
    {"songlyrics",  "s", "songlyrics",  false, {lyrics_songlyrics_parse,   lyrics_songlyrics_url,   NULL, true }, GRP_SPCL | GRP_SLOW},
    {"darklyrics",  "y", "darklyrics",  false, {lyrics_darklyrics_parse,   lyrics_darklyrics_url,   DARE, false}, GRP_SAFE | GRP_FAST},
    {"metrolyrics", "m", "metrolyrics", false, {lyrics_metrolyrics_parse,  lyrics_metrolyrics_url,  NULL, false}, GRP_SPCL | GRP_SLOW},
    { NULL,         NULL, NULL,         false, {NULL,                      NULL,                    NULL, false}, GRP_NONE | GRP_NONE}
};

GlyPlugin * glyr_get_lyric_providers(void)
{
    return copy_table(lyric_providers,sizeof(lyric_providers));
}

static GlyCacheList * lyrics_finalize(GlyCacheList * result, GlyQuery * settings)
{
    // no cleanup needs to be done
    if(!result) return NULL;

    GlyCacheList * lst = DL_new_lst();

    size_t i = 0;
    for(i = 0; i < result->size; i++)
    {
        GlyMemCache * dl = DL_init();
        dl->data = beautify_lyrics(result->list[i]->data);
        dl->size = strlen(dl->data);
        dl->dsrc = strdup(result->list[i]->dsrc);
        dl->type = TYPE_LYRICS;

        // call user defined callback
        if(settings->callback.download)
        {
            lst->usersig = settings->callback.download(dl,settings);
        }

        if(lst->usersig == GLYRE_OK)
        {
            DL_add_to_list(lst,dl);
        }
        else if(lst->usersig == GLYRE_STOP_BY_CB)
        {
            DL_free(dl);
            break;
        }
    }
    return lst;
}

GlyCacheList * get_lyrics(GlyQuery * settings)
{
    GlyCacheList * result = NULL;
    if(settings && settings->artist && settings->title)
    {
        result = register_and_execute(settings, lyrics_finalize);
    }
    else
    {
        glyr_message(2,settings,stderr,C_R"* "C_"%s is needed to download lyrics.\n",settings->artist ? "Title" : "Artist");
    }
    return result;
}
