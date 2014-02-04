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

#include "generic.h"
#include "../core.h"
#include "../stringlib.h"

struct callback_save_struct
{
    GHashTable * table;
    GLYR_DATA_TYPE type;
    GList * results;
};

/////////////////////////////////

/* Simple finalizer template sufficient for most */
GList * generic_txt_finalizer (GlyrQuery * settings, GList * input_list, gboolean * stop_me, GLYR_DATA_TYPE type, GList ** result_list)
{
    gboolean add_to_list = TRUE;
    GList * almost_copied = NULL;
    for (GList * elem = input_list; elem; elem = elem->next)
    {
        GlyrMemCache * item = elem->data;
        if (is_in_result_list (item,result_list[0]) == FALSE && add_to_list == TRUE)
        {
            /* Set to some default type */
            if (item->type == GLYR_TYPE_UNKNOWN)
            {
                item->type = type;
            }

            /* call user defined callback */
            GLYR_ERROR response = GLYRE_OK;
            if (settings->callback.download)
            {
                /* Call the usercallback */
                response = settings->callback.download (item,settings);
            }

            if (response != GLYRE_SKIP && response != GLYRE_STOP_PRE)
            {
                almost_copied = g_list_prepend (almost_copied,item);
            }
            else
            {
                DL_free (item);
                item = NULL;
            }

            if (response == GLYRE_STOP_POST || response == GLYRE_STOP_PRE)
            {
                add_to_list = FALSE;
                *stop_me = TRUE;
            }
        }
        else
        {
            settings->itemctr--;
            DL_free (item);
            item = NULL;
        }
    }
    return almost_copied;
}

/////////////////////////////////

static GList * async_dl_callback (cb_object * capo, void * userptr, bool * stop_download, gint * add_item)
{
    if (capo->cache != NULL && userptr != NULL)
    {
        /* Sanitize this */
        struct callback_save_struct * saver = userptr;
        GHashTable * prov_url_table = saver->table;
        capo->cache->is_image = true;

        if (prov_url_table != NULL)
        {
            GlyrMemCache * old_cache = g_hash_table_lookup (prov_url_table,capo->cache->dsrc);
            GLYR_ERROR response = GLYRE_OK;
            if (old_cache != NULL)
            {
                update_md5sum (capo->cache);
                if (is_in_result_list (capo->cache,saver->results) == FALSE)
                {
                    capo->cache->prov       = (old_cache->prov!=NULL) ? g_strdup (old_cache->prov) : NULL;
                    capo->cache->img_format = (old_cache->img_format) ? g_strdup (old_cache->img_format) : NULL;

                    if (capo->cache->type == GLYR_TYPE_UNKNOWN)
                    {
                        capo->cache->type = saver->type;
                    }

                    if (capo->s->callback.download != NULL)
                    {
                        response = capo->s->callback.download (capo->cache,capo->s);
                    }

                    *add_item = (response != GLYRE_SKIP && response != GLYRE_STOP_PRE);
                }
                else
                {
                    capo->s->itemctr--;
                    *add_item = FALSE;
                }
            }

            if (response == GLYRE_STOP_POST || response == GLYRE_STOP_PRE)
            {
                *stop_download = TRUE;
            }
        }
        else
        {
            glyr_message (-1,NULL,"glyr: Warn: Hashtable is empty im image_callback!\n");
        }
    }
    return NULL;
}

/////////////////////////////////

GList * generic_img_finalizer (GlyrQuery * s, GList * list, gboolean * stop_me, GLYR_DATA_TYPE type, GList ** result_list)
{
    /* Just return URLs */
    if (s->download == false)
    {
        for (GList * elem = list; elem; elem = elem->next)
        {
            GlyrMemCache * img = elem->data;
            img->is_image = false;
        }
        return generic_txt_finalizer (s,list,stop_me,GLYR_TYPE_IMG_URL,result_list);
    }
    else
    {
        /* Convert to a list of URLs first */
        GList * url_list  = NULL;

        /* Hashtable to associate the provider name with the corresponding URL */
        GHashTable * cache_url_table = g_hash_table_new_full (g_str_hash,g_str_equal,
                                       NULL,
                                       (GDestroyNotify) DL_free
                                                             );

        /* Iterate over all caches and turn them to GList */
        for (GList * item = list; item; item = item->next)
        {
            GlyrMemCache * cache = item->data;

            /* Make a copy, since we free the cache */
            gchar * url_double = g_strdup (cache->data);
            url_list  = g_list_prepend (url_list,url_double);

            /* Fill in the URL */
            g_hash_table_insert (cache_url_table, (gpointer) url_double, (gpointer) cache);
        }

        /* We need to pass this to the callback */
        struct callback_save_struct userptr =
        {
            .table = cache_url_table,
            .type  = type,
            .results = result_list ? result_list[0] : NULL
        };

        /* Download images in parallel */
        GList * dl_raw_images = async_download (url_list,NULL,s,1, (g_list_length (url_list) /2),async_dl_callback,&userptr,FALSE);

        /* Default to the given type */
        for (GList * elem = dl_raw_images; elem; elem = elem->next)
        {
            GlyrMemCache * item = elem->data;
            if (item && item->type == GLYR_TYPE_UNKNOWN)
            {
                item->type = type;
            }
        }

        /* Freeing Party */
        g_hash_table_destroy (cache_url_table);
        glist_free_full (url_list,g_free);

        /* Ready to save images */
        return dl_raw_images;
    }
    return NULL;
}

/////////////////////////////////
