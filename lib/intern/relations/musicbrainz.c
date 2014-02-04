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
#include "../common/musicbrainz.h"

/////////////////////////////////

#define RELATION_TARGLYR_GET_TYPE "<relation-list target-type=\"Url\">"
#define RELATION_BEGIN_TYPE  "<relation"

/* Wrap around the (a bit more) generic versions */
static GList * relations_musicbrainz_parse (cb_object * capo)
{
    GList * results = NULL;
    gint mbid_marker = 0;
    while (continue_search (g_list_length (results), capo->s) )
    {
        GlyrMemCache  * infobuf = generic_musicbrainz_parse (capo,&mbid_marker,"url-rels");
        if (infobuf == NULL)
        {
            break;
        }
        gsize nlen = (sizeof RELATION_BEGIN_TYPE) - 1;
        gchar * node = strstr (infobuf->data,RELATION_TARGLYR_GET_TYPE);
        if (node != NULL)
        {
            gint ctr = 0;
            while (continue_search (ctr,capo->s) && (node = strstr (node+nlen,RELATION_BEGIN_TYPE) ) )
            {
                node += nlen;
                gchar * target = get_search_value (node,"target=\"","\"");
                gchar * type   = get_search_value (node,"type=\"","\"");

                if (type != NULL && target != NULL)
                {
                    GlyrMemCache * tmp = DL_init();
                    tmp->data = g_strdup_printf ("%s:%s",type,target);
                    tmp->size = strlen (tmp->data);
                    tmp->dsrc = g_strdup (infobuf->dsrc);
                    results = g_list_prepend (results,tmp);
                    ctr++;

                    g_free (type);
                    g_free (target);
                }
            }
        }
        DL_free (infobuf);
    }
    return results;
}

/////////////////////////////////

static const gchar * relations_musicbrainz_url (GlyrQuery * sets)
{
    return generic_musicbrainz_url (sets);
}

/////////////////////////////////

MetaDataSource relations_musicbrainz_src =
{
    .name = "musicbrainz",
    .key  = 'm',
    .parser    = relations_musicbrainz_parse,
    .get_url   = relations_musicbrainz_url,
    .type      = GLYR_GET_RELATIONS,
    .quality   = 80,
    .speed     = 80,
    .endmarker = NULL,
    .free_url  = true
};
