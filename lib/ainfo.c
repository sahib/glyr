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
#include "stringlib.h"

#include "ainfo/lastfm.h"
#include "ainfo/allmusic_com.h"
#include "ainfo/wikipedia.h" // currently disabled
#include "ainfo/lyricsreg.h"

#define WKED "<ol class=\"references\">"
//#define LGED "<div class=\"sidebar\">"
#define LGED NULL


// Add your's here
GlyPlugin ainfo_providers[] =
{
//  full name       key  coloredname          use?   parser callback           geturl callback         free url?
    {"lastfm",      "l", "last"C_R"."C_"fm",  false,  {ainfo_lastfm_parse,     ainfo_lastfm_url,       NULL, true }, GRP_SAFE | GRP_FAST},
//  {"wikipedia",   "w", "wikipedia",         false,  {ainfo_wikipedia_parse,  ainfo_wikipedia_url,    WKED, false}, GRP_SAFE | GRP_FAST}, // not working yet.
    {"allmusic",    "m", C_"all"C_C"music",   false,  {ainfo_allmusic_parse,   ainfo_allmusic_url,     NULL, false}, GRP_USFE | GRP_SLOW},
    {"lyricsreg",   "r", C_"lyricsreg",       false,  {ainfo_lyricsreg_parse,  ainfo_lyricsreg_url,    LGED, false}, GRP_USFE | GRP_FAST},
    { NULL,         NULL,NULL,                false,  {NULL,                   NULL,                   NULL, false}, GRP_NONE | GRP_NONE},
};

GlyPlugin * glyr_get_ainfo_providers(void)
{
    return copy_table(ainfo_providers,sizeof(ainfo_providers));
}

static GlyCacheList * ainfo_finalize(GlyCacheList * result, GlyQuery * settings)
{
    return generic_finalizer(result,settings,TYPE_AINFO);
}

GlyCacheList * get_ainfo(GlyQuery * settings)
{
    GlyCacheList * result = NULL;
    if(settings && settings->artist)
    {
        result = register_and_execute(settings, ainfo_finalize);
    }
    else
    {
        glyr_message(2,settings,stderr,C_R":: "C_"Artist is needed to download artist description.\n");
    }
    return result;
}
