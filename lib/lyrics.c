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

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "core.h"
#include "types.h"
#include "stringop.h"

#include "lyrics/lyricswiki.h"
#include "lyrics/magistrix.h"
#include "lyrics/lyrix_at.h"
#include "lyrics/darklyrics.h"
#include "lyrics/lyricsvip.h"
#include "lyrics/directlyrics.h"
#include "lyrics/songlyrics.h"
#include "lyrics/lyrdb.h"
#include "lyrics/metrolyrics.h"

// Add your's here
GlyPlugin lyric_providers[] =
{
//  full name       key  coloredname    use?    parser callback           geturl callback         free url?
    {"lyricswiki",  "w", "lyricswiki",  false, {lyrics_lyricswiki_parse,  lyrics_lyricswiki_url,  false}},
    {"lyr.db",      "d", "lyr.db",      false, {lyrics_lyrdb_parse,       lyrics_lyrdb_url,       false}},
    {"lyrix.at",    "a", "lyrix.at",    false, {lyrics_lyrixat_parse,     lyrics_lyrixat_url,     false}},
    {"safe",        NULL,NULL,          false, {NULL,                     NULL,                   false}},
    {"magistrix",   "x", "magistrix",   false, {lyrics_magistrix_parse,   lyrics_magistrix_url,   false}},
    {"directlyrics","i", "directlyrics",false, {lyrics_directlyrics_parse,lyrics_directlyrics_url,true }},
    {"lyricsvip",   "v", "lyricsvip",   false, {lyrics_lyricsvip_parse,   lyrics_lyricsvip_url,   true }},
    {"unsafe",      NULL, NULL,         false, {NULL,                     NULL,                   false}},
    {"songlyrics",  "s", "songlyrics",  false, {lyrics_songlyrics_parse,  lyrics_songlyrics_url,  true }},
    {"darklyrics",  "y", "darklyrics",  false, {lyrics_darklyrics_parse,  lyrics_darklyrics_url,  false}},
    {"metrolyrics", "m", "metrolyrics", false, {lyrics_metrolyrics_parse, lyrics_metrolyrics_url, false}},
    {"special",     NULL, NULL,         false, {NULL,                     NULL,                   false}},
    { NULL,         NULL, NULL,         false, {NULL,                     NULL,                   false}},
};

GlyPlugin * glyr_get_lyric_providers(void)
{
    return copy_table(lyric_providers,sizeof(lyric_providers));
}

static GlyCacheList * lyrics_finalize(GlyCacheList * result, GlyQuery * settings)
{
    if(!result) return NULL;

    GlyCacheList * lst = DL_new_lst();

    size_t i = 0;
    for(i = 0; i < result->size; i++)
    {
        GlyMemCache * dl = DL_init();
        dl->data = beautify_lyrics(result->list[i]->data);
        dl->size = strlen(dl->data);
        dl->dsrc = strdup(result->list[i]->dsrc);

        // call user defined callback
        if(settings->callback.download)
            settings->callback.download(result->list[i],settings);

        DL_add_to_list(lst,dl);
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
