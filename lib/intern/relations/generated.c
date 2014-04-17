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
#include "../../stringlib.h"
#include "../../core.h"

/////////////////////////////////

static GList *relations_generated_parse(cb_object *capo)
{
    GList *result_list = NULL;
    GList *temple_list = NULL;

    if(capo->s->title && capo->s->album) {
        temple_list = g_list_prepend(temple_list, "Musicbrainz Recording:http://musicbrainz.org/search?query=\"${title}\"+AND+artist%3A\"${artist}\"+AND+release%3A\"${album}\"&type=recording&advanced=1");
    }

    if(capo->s->album) {
        temple_list = g_list_prepend(temple_list, "Musicbrainz Album:http://musicbrainz.org/search?query=\"${album}\"+AND+artist%3A\"${artist}\"&type=release&advanced=1");
        temple_list = g_list_prepend(temple_list, "Wikipedia Album:http://en.wikipedia.org/wiki/Special:Search/${album}(${artist})");
    }

    temple_list = g_list_prepend(temple_list, "Musicbrainz Artist:http://musicbrainz.org/search?query=\"${artist}\"&type=artist");
    temple_list = g_list_prepend(temple_list, "Wikipedia Artist:http://en.wikipedia.org/wiki/Special:Search/${artist}");
    temple_list = g_list_prepend(temple_list, "Lastfm Page:http://last.fm/music/${artist}");

    for(GList *elem = temple_list; elem; elem = elem->next) {
        gchar *result_url = prepare_url(elem->data, capo->s, TRUE);
        if(result_url != NULL) {
            GlyrMemCache *result = DL_init();
            result->data = result_url;
            result->size = strlen(result_url);
            result->dsrc = g_strdup(OFFLINE_PROVIDER);
            result->prov = g_strdup("generated");
            result->type = GLYR_TYPE_RELATION;
            update_md5sum(result);
            result_list = g_list_prepend(result_list, result);
        }
    }

    g_list_free(temple_list);
    return result_list;
}

/////////////////////////////////

static const gchar *relations_generated_url(GlyrQuery *sets)
{
    return OFFLINE_PROVIDER;
}

/////////////////////////////////

MetaDataSource relations_generated_src = {
    .name = "generated",
    .key  = 'g',
    .parser    = relations_generated_parse,
    .get_url   = relations_generated_url,
    .type      = GLYR_GET_RELATIONS,
    .quality   = 100,
    .speed     = 100,
    .endmarker = NULL,
    .free_url  = false
};
