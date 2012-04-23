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

#include <string.h>
#include "autohelp.h"

#include "../../lib/stringlib.h"

/* ------------------------------------------------------------- */

void suggest_other_getter(GlyrQuery * query, const char * wrong_input)
{
	if(query->verbosity <= 0)
	{
		return;
	}

	GlyrFetcherInfo * fetcher = glyr_info_get();
	if(fetcher != NULL)
	{
		gboolean did_you_mean_printed = FALSE;
		GlyrFetcherInfo * it = fetcher;
		while(it != NULL)
		{
			if(levenshtein_strcasecmp(wrong_input,it->name) < 7)
			{
				if(did_you_mean_printed == FALSE)
				{
					g_print("\nDid you mean this?\n");
					did_you_mean_printed = TRUE;
				}
				g_print(" - %s\n",it->name);
			}		
			it = it->next;
		}
		glyr_info_free(fetcher);
	}
}

/*-----------------------------------------*/

void suggest_other_provider(GlyrQuery * query, const char * wrong_input)
{
	if(query->verbosity <= 0)
	{
		return;
	}

	GlyrFetcherInfo * fetcher = glyr_info_get();
	GlyrFetcherInfo * it = fetcher;
	while(it != NULL)
	{
		if(it->type == query->type)
		{
			break;
		}

		it = it->next;
	}

	if(it != NULL && wrong_input)
	{
		gboolean did_you_mean_printed = FALSE;
		GlyrSourceInfo * head = it->head;
		GHashTable * key_table = g_hash_table_new(g_direct_hash,g_direct_equal);
		while(head != NULL)
		{
			gsize offset = 0;
			gsize length = strlen(wrong_input);
			gchar *token = NULL;
			while((token = get_next_word(wrong_input,GLYR_DEFAULT_FROM_ARGUMENT_DELIM,&offset,length)) != NULL)
			{	
				if(levenshtein_strcasecmp(token,head->name) < 5 &&
						g_hash_table_lookup(key_table,head->name) == NULL)
				{
					if(did_you_mean_printed == FALSE)
					{
						g_print("\nDid you mean this?\n");
						did_you_mean_printed = TRUE;
					}
					g_print(" - %s\n",head->name);
					g_hash_table_insert(key_table,head->name,head);
				}
				g_free(token);
			}
			head = head->next;	
		}
		g_hash_table_destroy(key_table);
	}		
	glyr_info_free(fetcher);
}

/*-----------------------------------------*/

