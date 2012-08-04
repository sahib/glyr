/***********************************************************
 * This file is part of glyr
 * + a commnandline tool and library to download various sort of musicrelated metadata.
 * + Copyright (C) [2011-2012]  [Christopher Pahl]
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

/* register all plugins here */
#include "core.h"
#include "register_plugins.h"

/* Warning: All functions in here are _not_ threadsafe. */
static void get_list_from_type (MetaDataFetcher * fetch);
static void register_provider_plugins (void);
static void init_provider_list (void);

/* global provider plugin list */
GList * glyrMetaDataSourceList = NULL;

/* List of MetaDataFetchers */
GList * glyrMetaDataPluginList = NULL;


/* Externalized fetcher vars, add yours here  {{{ */
extern MetaDataFetcher glyrFetcher_cover;
extern MetaDataFetcher glyrFetcher_lyrics;
extern MetaDataFetcher glyrFetcher_artistphotos;
extern MetaDataFetcher glyrFetcher_artistbio;
extern MetaDataFetcher glyrFetcher_similar_artists;
extern MetaDataFetcher glyrFetcher_similar_song;
extern MetaDataFetcher glyrFetcher_review;
extern MetaDataFetcher glyrFetcher_albumlist;
extern MetaDataFetcher glyrFetcher_tags;
extern MetaDataFetcher glyrFetcher_relations;
extern MetaDataFetcher glyrFetcher_tracklist;
extern MetaDataFetcher glyrFetcher_guitartabs;
extern MetaDataFetcher glyrFetcher_backdrops;
/* }}} */

/* Externalized sourceprovider vars, add yours here {{{ */
extern MetaDataSource ainfo_lastfm_src;
extern MetaDataSource ainfo_lyricsreg_src;
extern MetaDataSource ainfo_bbcmusic_src;
extern MetaDataSource albumlist_musicbrainz_src;
extern MetaDataSource cover_coverartarchive_src;
extern MetaDataSource cover_albumart_src;
extern MetaDataSource cover_amazon_src;
extern MetaDataSource cover_coverhunt_src;
extern MetaDataSource cover_discogs_src;
extern MetaDataSource cover_google_src;
extern MetaDataSource cover_lastfm_src;
extern MetaDataSource cover_jamendo_src;
extern MetaDataSource cover_lyricswiki_src;
extern MetaDataSource cover_rhapsody_src;
extern MetaDataSource cover_picsearch_src;
extern MetaDataSource cover_musicbrainz_src;
extern MetaDataSource cover_slothradio_src;
extern MetaDataSource lyrics_lipwalk_src;
extern MetaDataSource lyrics_lyrdb_src;
extern MetaDataSource lyrics_lyricsreg_src;
extern MetaDataSource lyrics_lyricstime_src;
extern MetaDataSource lyrics_lyricsvip_src;
extern MetaDataSource lyrics_lyricswiki_src;
extern MetaDataSource lyrics_lyrix_src;
extern MetaDataSource lyrics_magistrix_src;
extern MetaDataSource lyrics_metrolyrics_src;
extern MetaDataSource lyrics_metallum_src;
extern MetaDataSource lyrics_elyrics_src;
extern MetaDataSource lyrics_chartlyrics_src;
extern MetaDataSource photos_flickr_src;
extern MetaDataSource photos_google_src;
extern MetaDataSource photos_lastfm_src;
extern MetaDataSource photos_singerpictures_src;
extern MetaDataSource photos_discogs_src;
extern MetaDataSource photos_rhapsody_src;
extern MetaDataSource photos_picsearch_src;
extern MetaDataSource photos_bbcmusic_src;
extern MetaDataSource relations_musicbrainz_src;
extern MetaDataSource review_amazon_src;
extern MetaDataSource review_metallum_src;
extern MetaDataSource similar_artist_lastfm_src;
extern MetaDataSource similar_song_lastfm_src;
extern MetaDataSource tags_musicbrainz_src;
extern MetaDataSource tracklist_musicbrainz_src;
//extern MetaDataSource guitartabs_guitaretab_src;
extern MetaDataSource guitartabs_chordie_src;
extern MetaDataSource backdrops_htbackdrops_src;

/* Special offline providers */
extern MetaDataSource local_provider_src;
extern MetaDataSource relations_generated_src;
extern MetaDataSource musictree_provider_src;
/* }}} */

// Disabled due to bad quality.
//extern MetaDataSource lyrics_darklyrics_src;
//extern MetaDataSource lyrics_directlyrics_src;
//extern MetaDataSource lyrics_songlyrics_src;
/////////////////////////////////

/* Get fetcher list */
GList * r_getFList (void)
{
    return glyrMetaDataPluginList;
}

/////////////////////////////////

/* Get Source list */
GList * r_getSList (void)
{
    return glyrMetaDataSourceList;
}

/////////////////////////////////

void plugin_add_to_list (GList ** list, void * data)
{
    if (list != NULL)
    {
        *list = g_list_prepend (*list,data);
    }
}

/////////////////////////////////

static void register_provider_plugins (void)
{
    plugin_add_to_list (&glyrMetaDataSourceList,&ainfo_lastfm_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&ainfo_lyricsreg_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&ainfo_bbcmusic_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&albumlist_musicbrainz_src);

    // FIXME: Seems to be broken too. (serverside)
    //plugin_add_to_list(&glyrMetaDataSourceList,&cover_albumart_src);

    // FIXME: Silly amazon requires to be a Seller to use their API
    // plugin_add_to_list(&glyrMetaDataSourceList,&cover_amazon_src);

    // FIXME: Coverhunt seems to be down. Add again when up and running
    //plugin_add_to_list(&glyrMetaDataSourceList,&cover_coverhunt_src);


    // FIXME: No real data yet there.. adele:19 gives a correct mbid, but no results
    //plugin_add_to_list(&glyrMetaDataSourceList,&cover_coverartarchive_src);

    plugin_add_to_list (&glyrMetaDataSourceList,&cover_discogs_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&cover_google_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&cover_lastfm_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&cover_jamendo_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&cover_lyricswiki_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&cover_rhapsody_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&cover_picsearch_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&cover_musicbrainz_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&cover_slothradio_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&lyrics_lipwalk_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&lyrics_lyrdb_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&lyrics_lyricsreg_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&lyrics_lyricstime_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&lyrics_lyricsvip_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&lyrics_lyricswiki_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&lyrics_lyrix_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&lyrics_magistrix_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&lyrics_metrolyrics_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&lyrics_metallum_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&lyrics_elyrics_src);

    // FIXME: Chartlyrics reacts very slowly, and often not at all
    // add again when running again ( :( - good api though)
    // 28.1.2012: Seems to be on again, API fails too often though still.
    plugin_add_to_list (&glyrMetaDataSourceList,&lyrics_chartlyrics_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&photos_flickr_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&photos_google_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&photos_lastfm_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&photos_discogs_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&photos_singerpictures_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&photos_rhapsody_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&photos_picsearch_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&photos_bbcmusic_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&relations_musicbrainz_src);

    // FIXME: Silly amazon requires to be a Seller to use their API
    //plugin_add_to_list(&glyrMetaDataSourceList,&review_amazon_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&review_metallum_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&similar_artist_lastfm_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&similar_song_lastfm_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&tags_musicbrainz_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&tracklist_musicbrainz_src);
    // FIXME: Searchfunction of guitaretabs seems to be broken
    //plugin_add_to_list(&glyrMetaDataSourceList,&guitartabs_guitaretab_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&guitartabs_chordie_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&backdrops_htbackdrops_src);

    /* Special offline providers */
    plugin_add_to_list (&glyrMetaDataSourceList,&relations_generated_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&local_provider_src);
    plugin_add_to_list (&glyrMetaDataSourceList,&musictree_provider_src);
}

/////////////////////////////////

/* Just presort a bit, so we don't need to shuffle the plugins always *
 * This has no actual practical use, it's just that pretty plugins are listed
 * first in outputs. Actual sorting is not threadsafe!
 */
static gint compare_by_priority (gconstpointer a, gconstpointer b)
{
    const MetaDataSource * sa = a;
    const MetaDataSource * sb = b;
    return (sa != NULL && sb != NULL) ?
           (sa->quality * sa->speed -
            sb->quality * sb->speed
           ) : 0;
}

/////////////////////////////////

static void get_list_from_type (MetaDataFetcher * fetch)
{
    GList * src;
    for (src = glyrMetaDataSourceList; src; src = src->next)
    {
        MetaDataSource * item = src->data;
        if (item && (fetch->type == item->type || item->type == GLYR_GET_ANY) )
        {
            fetch->provider = g_list_prepend (fetch->provider,item);
            if (item->data_type == GLYR_TYPE_UNKNOWN)
            {
                item->data_type = fetch->default_data_type;
            }
        }

    }
    fetch->provider = g_list_sort (fetch->provider,compare_by_priority);

}

/////////////////////////////////

static void init_provider_list (void)
{
    for (GList * fetch = glyrMetaDataPluginList; fetch; fetch = fetch->next)
        get_list_from_type ( (MetaDataFetcher *) (fetch->data) );
}

/////////////////////////////////

/* Register fetchers */
void register_fetcher_plugins (void)
{
    /* add yours here */
    plugin_add_to_list (&glyrMetaDataPluginList,&glyrFetcher_cover);
    plugin_add_to_list (&glyrMetaDataPluginList,&glyrFetcher_lyrics);
    plugin_add_to_list (&glyrMetaDataPluginList,&glyrFetcher_artistphotos);
    plugin_add_to_list (&glyrMetaDataPluginList,&glyrFetcher_artistbio);
    plugin_add_to_list (&glyrMetaDataPluginList,&glyrFetcher_similar_artists);
    plugin_add_to_list (&glyrMetaDataPluginList,&glyrFetcher_similar_song);
    plugin_add_to_list (&glyrMetaDataPluginList,&glyrFetcher_review);
    plugin_add_to_list (&glyrMetaDataPluginList,&glyrFetcher_albumlist);
    plugin_add_to_list (&glyrMetaDataPluginList,&glyrFetcher_tags);
    plugin_add_to_list (&glyrMetaDataPluginList,&glyrFetcher_relations);
    plugin_add_to_list (&glyrMetaDataPluginList,&glyrFetcher_tracklist);
    plugin_add_to_list (&glyrMetaDataPluginList,&glyrFetcher_guitartabs);
    plugin_add_to_list (&glyrMetaDataPluginList,&glyrFetcher_backdrops);

    glyrMetaDataPluginList = g_list_reverse (glyrMetaDataPluginList);

    /* call init() */
    for (GList * elem = glyrMetaDataPluginList; elem; elem = elem->next)
    {
        MetaDataFetcher * fetch = elem->data;
        if (fetch->init != NULL)
        {
            fetch->init();
        }
    }

    /* le plugins */
    register_provider_plugins();

    init_provider_list();
}

/////////////////////////////////

/* Unregister 'em again */
void unregister_fetcher_plugins (void)
{
    /* Destroy all fetchers */
    if (glyrMetaDataPluginList)
    {
        GList * elem;
        for (elem = glyrMetaDataPluginList; elem != NULL; elem = elem->next)
        {
            MetaDataFetcher * item = elem->data;
            if (item->destroy != NULL)
            {
                item->destroy();
            }
            g_list_free (item->provider);
            item->provider = NULL;
        }
        g_list_free (glyrMetaDataPluginList);
        glyrMetaDataPluginList = NULL;
    }

    /* Also kill others */
    if (!glyrMetaDataSourceList)
    {
        g_list_free (glyrMetaDataSourceList);
        glyrMetaDataSourceList = NULL;
    }
}

/////////////////////////////////

GlyrFetcherInfo * get_plugin_info (void)
{
    GlyrFetcherInfo * head = NULL;
    GlyrFetcherInfo * prev_fetcher = NULL;
    for (GList * elem0 = r_getFList(); elem0; elem0 = elem0->next)
    {
        MetaDataFetcher * fetch = elem0->data;
        GlyrFetcherInfo  * finfo = g_malloc0 (sizeof (GlyrFetcherInfo) );

        GlyrSourceInfo * prev_source = NULL;
        for (GList * elem1 = r_getSList(); elem1; elem1 = elem1->next)
        {
            MetaDataSource * source = elem1->data;
            if (source && source->type == fetch->type)
            {
                GlyrSourceInfo  * sinfos = g_malloc0 (sizeof (GlyrSourceInfo) );

                sinfos->quality = source->quality;
                sinfos->speed   = source->speed;
                sinfos->key     = source->key;
                sinfos->type    = source->type;
                sinfos->lang_aware = source->lang_aware;
                sinfos->name    = g_strdup (source->name);

                if (prev_source != NULL)
                {
                    prev_source->next = sinfos;
                    sinfos->prev = prev_source;
                }
                else
                {
                    finfo->head = sinfos;
                }
                prev_source = sinfos;
            }
        }

        if (prev_fetcher != NULL)
        {
            prev_fetcher->next = finfo;
            finfo->prev = prev_fetcher;
        }
        else
        {
            head = finfo;
        }

        prev_fetcher = finfo;

        finfo->name = g_strdup (fetch->name);
        finfo->type = fetch->type;
        finfo->reqs = fetch->reqs;
    }
    return head;
}

/////////////////////////////////

static void free_single_item (GlyrFetcherInfo * info)
{
    if (info != NULL)
    {
        GlyrSourceInfo * elem = info->head;
        while (elem != NULL)
        {
            GlyrSourceInfo * to_delete = elem;
            elem = elem->next;

            g_free (to_delete->name);
            g_free (to_delete);
        }
        g_free ( (gchar*) info->name);
        info->name = NULL;
    }
    g_free (info);
}

/////////////////////////////////

void free_plugin_info (GlyrFetcherInfo * infos)
{
    if (infos != NULL)
    {
        GlyrFetcherInfo * fetch = infos;
        while (fetch != NULL)
        {
            GlyrFetcherInfo * to_delete = fetch;
            fetch = fetch->next;
            free_single_item (to_delete);
        }
    }
}
