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

#include "core.h"
#include "stringlib.h"

/* ------------------------------------- */
#if 0
static GlyCacheList * lyrics_finalize(GlyCacheList * result, GlyQuery * settings)
{
	// no cleanup needs to be done
	if(!result) return NULL;

	GlyCacheList * lst = DL_new_lst();

	size_t i = 0;
	for(i = 0; i < result->size; i++)
	{
		GlyMemCache * dl = DL_copy(result->list[i]);
		if(dl != NULL)
		{
			if(dl->data)
			{
				free(dl->data);
			}
			if(dl->dsrc)
			{
				free(dl->dsrc);
			}
			if(dl->prov)
			{
				free(dl->prov);
			}

			dl->data = beautify_lyrics(result->list[i]->data);
			dl->size = strlen(dl->data);
			dl->dsrc = strdup(result->list[i]->dsrc);
			dl->type = TYPE_LYRICS;

			if(result->list[i]->prov)
			{
				dl->prov = strdup(result->list[i]->prov);
			}

			// call user defined callback
			if(settings->callback.download)
			{
				lst->usersig = settings->callback.download(dl,settings);
			}

			if(lst->usersig == GLYRE_OK)
			{
				DL_add_to_list(lst,dl);
			}
			else if(lst->usersig == GLYRE_STOP_BY_CB)
			{
				DL_free(dl);
				break;
			}
		}
	}
	return lst;
}
#endif

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

GList * factory(GlyQuery * s, GList * query) { return NULL; }

/* ------------------------------------- */

/* PlugStruct */
MetaDataFetcher glyrFetcher_lyrics = {
	.name = "Lyrics Fetcher",
	.type = GET_LYRICS,
	.validate  = vdt_lyrics,
	.init    = NULL,
	.destroy = NULL,
	.finalize = factory
};

/* ------------------------------------- */
