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
#include "../../cache.h"

/////////////////////////////////

static GList * local_provider_parse (cb_object * capo)
{
    GList * converter_list = NULL;
    if (capo->s && capo->s->local_db != NULL)
    {
        gint counter = 0;
        GlyrMemCache * head = glyr_db_lookup (capo->s->local_db,capo->s);
        while (head != NULL)
        {
            if (counter < capo->s->number)
            {
                converter_list = g_list_prepend (converter_list,head);
                head->cached = TRUE;
                head = head->next;
                counter++;
            }
            else
            {
                GlyrMemCache * to_delete = head;
                head = head->next;
                DL_free (to_delete);
            }
        }
    }
    return converter_list;
}

/////////////////////////////////

static const gchar * local_provider_url (GlyrQuery * sets)
{
    return OFFLINE_PROVIDER;
}

/////////////////////////////////

MetaDataSource local_provider_src =
{
    .name = "local",
    .key  = 'l',
    .parser    = local_provider_parse,
    .get_url   = local_provider_url,
    .type      = GLYR_GET_ANY,
    .quality   = 4200,
    .speed     = 4200,
    .endmarker = NULL,
    .free_url  = false
};
