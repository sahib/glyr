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

#include "core.h"
#include "stringlib.h"



/* ------------------------------------- */

#if 0
static GlyCacheList * cover_callback(cb_object * capo)
{
	// in this 'pseudo' callback we copy
	// the downloaded cache, and add the source url
	GlyCacheList * ls = NULL;
	GlyMemCache  * dl = DL_copy(capo->cache);

	// Now extend some fields
	if(dl != NULL)
	{
		int usersig = GLYRE_OK;

		dl->dsrc = strdup(capo->url);
		if(dl->type == TYPE_NOIDEA)
			dl->type = TYPE_COVER;

		if(capo->provider_name)
			dl->prov = strdup(capo->provider_name);

		dl->is_image = true;

		// call user defined callback if any
		if(capo->s->callback.download)
		{
			usersig = capo->s->callback.download(dl,capo->s);
		}

		if(usersig == GLYRE_OK)
		{
			ls = DL_new_lst();
			ls->usersig = usersig;
			DL_add_to_list(ls,dl);
		}
		else if(usersig == GLYRE_STOP_BY_CB)
		{
			DL_free(dl);
		}
	}
	return ls;
}

/* ------------------------------------- */

// Place any bad URLs that should never ever get out
// Prefer to handle this in the plugin though!
const char * URLblacklist[] =
{
	"http://ecx.images-amazon.com/images/I/11J2DMYABHL.jpg", // blank image.
	NULL
};

GlyCacheList * cover_finalize(GlyCacheList * result, GlyQuery * settings)
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
				flag_lint(result,settings);

				/* Watch out for blacklisted URLs */
				flag_blacklisted_urls(result,URLblacklist,settings);

				flag_invalid_format(result,settings);

				size_t i = 0;
				int ctr = 0;
				for(i = 0; i < result->size; i++)
				{
					if(!result->list[i]->error)
					{
						plugin_init(&urlplug_list[ctr], result->list[i]->data, cover_callback, settings, NULL, NULL, result->list[i]->prov, true);
						ctr++;
					}
				}

				dl_list = invoke(urlplug_list,ctr,settings->parallel,settings->timeout * ctr, settings);
				if(dl_list != NULL)
				{
					glyr_message(2,settings,stderr,"- Succesfully downloaded %d image(s).\n",dl_list->size);
				}
				free(urlplug_list);
			}
		}
		else
		{
			/* Ignore double URLs */
			flag_lint(result,settings);

			/* Watch out for blacklisted URLs */
			flag_blacklisted_urls(result,URLblacklist,settings);

			flag_invalid_format(result,settings);

			size_t i = 0;
			for( i = 0; i < result->size; i++)
			{
				if(result->list[i] && result->list[i]->error == ALL_OK && result->list[i]->data)
				{
					if(!dl_list) dl_list = DL_new_lst();

					GlyMemCache * copy = DL_init();
					copy->is_image = true;
					copy->type = TYPE_COVER;
					copy->dsrc = strdup(result->list[i]->data);
					copy->size = copy->dsrc ? strlen(copy->dsrc) : 0;

					if(settings->callback.download)
						dl_list->usersig = settings->callback.download(copy,settings);

					if(dl_list->usersig == GLYRE_OK)
					{
						DL_add_to_list(dl_list,copy);
					}
					else if(dl_list->usersig == GLYRE_STOP_BY_CB)
					{
						DL_free(copy);
						break;
					}
				}
			}
		}
	}
	return dl_list;
}
#endif

static GList * factory(GlyQuery * s, GList * list) {
	/* Just return URLs */
	if(s->download == false)
	{
		for(GList * elem = list; elem; elem = elem->next) {
			GlyMemCache * img = elem->data;
			img->is_image = true;
			img->type = TYPE_IMG_URL;
		}	
		return list;

	} else {
		/* Convert to a list of URLs first */
		GList * url_list = NULL; 
		for(GList * item = list; item; item = item->next) {
			GlyMemCache * cache = item->data;
			url_list = g_list_prepend(url_list,cache->data);
		}		

		GList * dl_raw_images = async_download(url_list,s,1,1,NULL,NULL);
		if(dl_raw_images != NULL) {
			for(GList * elem = dl_raw_images; elem; elem = elem->next)
			{
				GlyMemCache * img = elem->data;
				img->is_image = true;
				img->type = TYPE_COVER;
			}
		}
		return dl_raw_images;
	}
	return NULL;
}

/* ------------------------------------- */

bool vdt_cover(GlyQuery * settings)
{
	if (settings && settings->artist && settings->album)
	{
		// validate size
		if(settings->cover.min_size <= 0)
			settings->cover.min_size = -1;

		if(settings->cover.max_size <= 0)
			settings->cover.max_size = -1;

		return true;
	}
	else
	{
		return false;
	}
}

/* ------------------------------------- */

/* PlugStruct */
MetaDataFetcher glyrFetcher_cover = {
	.name = "Cover Fetcher",
	.type = GET_COVERART,
	.validate = vdt_cover,
	.init    = NULL,
	.destroy = NULL,
	.finalize = factory
};

/* ------------------------------------- */

