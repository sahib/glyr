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

#include "review/allmusic_com.h"
#include "review/amazon.h"

// Add your's here
GlyPlugin review_providers[] = {
//  full name       key   coloredname          use?   parser callback          geturl callback         stop download after this?     free url?
        {"allmusic",    "m",  C_"all"C_C"music"C_,false,  {review_allmusic_parse,  review_allmusic_url,    "<div id=\"right-sidebar\">", false},   GRP_SAFE | GRP_SLOW},
        {"amazon",      "a",  "amazon",           false,  {review_amazon_parse,    review_amazon_url,      NULL,                         true },   GRP_SAFE | GRP_FAST},
        { NULL,         NULL, NULL,               false,  {NULL,                   NULL,                   NULL,                         false},   GRP_NONE | GRP_NONE},
};

GlyPlugin * glyr_get_review_providers(void)
{
        return copy_table(review_providers,sizeof(review_providers));
}

static GlyCacheList * review_finalize(GlyCacheList * result, GlyQuery * settings)
{
        return generic_finalizer(result,settings,TYPE_REVIEW);
}

GlyCacheList * get_review(GlyQuery * settings)
{
        GlyCacheList * result = NULL;
        if(settings && settings->artist && settings->album) {
                result = register_and_execute(settings, review_finalize);
        } else {
                glyr_message(2,settings,stderr,C_R"* "C_"Artist and album is needed to retrieve reviews (o rly?).\n");
        }
        return result;
}
