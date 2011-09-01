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

/* The blacklist consists of two links at the moment :-P */
#include "blacklist.h"

/*-----------------------------------------------------------------*/

GHashTable * lookup_table = NULL;
gchar * blacklist_array[] =
{
	"http://ecx.images-amazon.com/images/I/11J2DMYABHL.jpg",       /* blank image */
	"http://cdn.recordshopx.com/cover/normal/5/53/53138.jpg%3Fcd"  /* blank image */
};

/*-----------------------------------------------------------------*/

void blacklist_build(void)
{
	lookup_table = g_hash_table_new(g_str_hash,g_str_equal);
	gint b_size = sizeof(blacklist_array) / sizeof(gchar *);
	for(gint it = 0; it < b_size; it++)
	{
		if(blacklist_array[it] != NULL)
		{
			g_hash_table_insert(lookup_table,blacklist_array[it],blacklist_array[it]);
		}
	}
}

/*-----------------------------------------------------------------*/

void blacklist_destroy(void)
{
	g_hash_table_destroy(lookup_table);
}

/*-----------------------------------------------------------------*/

gboolean is_blacklisted(gchar * URL)
{
	if(lookup_table == NULL || URL == NULL)
		return FALSE;

	return !(g_hash_table_lookup(lookup_table,URL) == NULL);
}

/*-----------------------------------------------------------------*/
