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

#include "../core.h"
#include "../stringlib.h"
#include "generic.h"

/* ------------------------------------- */

bool vdt_lyrics(GlyQuery * settings)
{
    if(settings && settings->artist && settings->title)
    {
        return true;
    }
    else
    {
        glyr_message(2,settings,stderr,C_R"* "C_"%s is needed to download lyrics.\n",settings->artist ? "Title" : "Artist");
        return false;
    }
}

/* ------------------------------------- */

GList * factory(GlyQuery * s, GList * list, gboolean * stop_me)
{
    /* Fix up lyrics, escape chars etc.  */
    for(GList * elem = list; elem; elem = elem->next)
    {
        GlyMemCache * item = elem->data;
        if(item != NULL)
        {

            gchar * temp = beautify_lyrics(item->data);
            g_free(item->data);
            item->data = temp;
            item->size = (item->data) ? strlen(item->data) : 0;
        }
    }

    /* Let the rest do by the norma generic finalizer */
    return generic_txt_finalizer(s,list,stop_me,TYPE_LYRICS);
}

/* ------------------------------------- */

/* PlugStruct */
MetaDataFetcher glyrFetcher_lyrics =
{
    .name = "lyrics",
    .type = GET_LYRICS,
    .validate  = vdt_lyrics,
    .full_data = TRUE,
    .init    = NULL,
    .destroy = NULL,
    .finalize = factory
};

/* ------------------------------------- */
