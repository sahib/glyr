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

/* Copied from lib/stringlib.c - more details there */
gsize levenshtein_strcmp(const gchar * s, const gchar * t)
{
 	 /* Get strlen */
    int n = (s) ? strlen(s)+1 : 0;
    int m = (t) ? strlen(t)+1 : 0;

    /* Nothing to compute really */
    if (n == 0) return m;
    if (m == 0) return n;

    /* String matrix */
    int d[n][m];
    int i,j;

    /* Init first row|column to 0...n|m */
    for (i = 0; i < n; i++) d[i][0] = i;
    for (j = 0; j < m; j++) d[0][j] = j;

    for (i = 1; i < n; i++)
    {
        /* Current char in string s */
        char cats = s[i-1];

        for (j = 1; j < m; j++)
        {
            /* Do -1 only once */
            int jm1 = j-1,
                im1 = i-1;

            /* a = above cell, b = left cell, c = left above cell */
            int a = d[im1][j] + 1,
                b = d[i][jm1] + 1,
                c = d[im1][jm1] + (t[jm1] != cats);

            /* Now compute the minimum of a,b,c and set MIN(a,b,c) to cell d[i][j] */
 	    d[i][j] = (a < b) ? MIN(a,c) : MIN(b,c);
        }
    }

    /* The result is stored in the very right down cell */
    return d[n-1][m-1];
}

/* ------------------------------------------------------------- */

gsize levenshtein_strcasecmp(const gchar * string, const gchar * other)
{
	gsize diff = 100;
	if(string != NULL && other != NULL)
	{
		/* Lowercase UTF8 string might have more or less bytes! */
		gchar * lower_string = g_ascii_strdown(string,-1);
		gchar * lower_other  = g_ascii_strdown(other, -1);

		if(lower_string && lower_other)
		{
			diff = levenshtein_strcmp(lower_string, lower_other);
		}

		/* Free 'em */
		g_free(lower_string);
		g_free(lower_other);
	}
	return diff;
}

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

