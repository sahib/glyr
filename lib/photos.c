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

#include "photos.h"
#include "core.h"
#include "types.h"
#include "stringop.h"

#include "photos/flickr.h"
#include "photos/lastfm.h"

GlyPlugin photos_providers[] =
{
    {"lastfm", "l",  C_"last"C_R"."C_"fm",  false, {photos_lastfm_parse, photos_lastfm_url, false}},
    {"safe",   NULL, NULL,                  false, {NULL,              NULL,                false}},
    {"flickr", "f",  C_C"flick"C_R"r",      false, {photos_flickr_parse, photos_flickr_url, true }},
    {"unsafe", NULL, NULL,                  false, {NULL,              NULL,                false}},
    {NULL,     NULL, NULL,                  false, {NULL,              NULL,                false}}
};

GlyPlugin * glyr_get_photo_providers(void)
{
    return copy_table(photos_providers,sizeof(photos_providers));
}

static GlyCacheList * cover_callback(cb_object * capo)
{
    GlyCacheList * ls = DL_new_lst();
    GlyMemCache * dl = DL_copy(capo->cache);
    if(dl)
    {
        dl->dsrc = strdup(capo->url);

        // call user defined callback
        if(capo->s->callback.download)
            capo->s->callback.download(dl,capo->s);

        DL_add_to_list(ls,dl);
    }
    return ls;
}

static GlyCacheList * photo_finalize(GlyCacheList * result, GlyQuery * settings)
{
    GlyCacheList * dl_list = NULL;
    if(result)
    {
        if(settings->download)
        {
            cb_object  * urlplug_list = calloc(result->size+1,sizeof(cb_object));
            if(urlplug_list)
            {
                /* Ignore double URLs */
                flag_double_urls(result,settings);

                size_t ctr = 0,i = 0;
                for(i = 0; i < result->size; i++)
                {
                    if(result->list[i] && result->list[i]->data && result->list[i]->error == ALL_OK)
                    {
                        plugin_init(&urlplug_list[ctr], result->list[i]->data, cover_callback, settings, NULL, NULL, NULL);
                        ctr++;
                    }

                    if(result->list[i]->data)
                    {
                        free(result->list[i]->data);
                    }
                }
                dl_list = invoke(urlplug_list,i,settings->parallel,settings->timeout * i, settings);
                free(urlplug_list);
            }
        }
        else
        {
            size_t i = 0;

            /* Ignore double URLs */
            flag_double_urls(result,settings);

            for( i = 0; i < result->size; i++)
            {
                if(result->list[i] && result->list[i]->error == ALL_OK)
                {
                    if(!dl_list) dl_list = DL_new_lst();

                    GlyMemCache * r_copy = DL_init();
                    r_copy->data = strdup(result->list[i]->data);
                    r_copy->size = strlen(r_copy->data);
                    DL_add_to_list(dl_list,r_copy);

                    free(result->list[i]->data);
                }
            }
        }
    }
    return dl_list;
}


GlyCacheList * get_photos(GlyQuery * settings)
{
    if (settings && settings->artist)
        return register_and_execute(settings,photo_finalize);
    else
        glyr_message(2,settings,stderr,C_R"[]"C_" Artist is needed to download artist-related photos!\n");

    return NULL;
}
