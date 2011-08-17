/* register all plugins here */
#include "core.h"
#include "register_plugins.h"

/* Warning: All functions in here are _not_ threadsafe. */
static void get_list_from_type(MetaDataFetcher * fetch);
static void register_provider_plugins(void);
static void init_provider_list(void);

/* global provider plugin list */
GList * glyrMetaDataSourceList = NULL;

/* List of MetaDataFetchers */
GList * glyrMetaDataPluginList = NULL;


/* Externalized fetcher vars, add yours here */
extern MetaDataFetcher glyrFetcher_cover;
extern MetaDataFetcher glyrFetcher_lyrics;
extern MetaDataFetcher glyrFetcher_artistphotos;
extern MetaDataFetcher glyrFetcher_artistbio;
extern MetaDataFetcher glyrFetcher_similiar_artists;
extern MetaDataFetcher glyrFetcher_similar_song;
extern MetaDataFetcher glyrFetcher_review;
extern MetaDataFetcher glyrFetcher_albumlist;
extern MetaDataFetcher glyrFetcher_tags;
extern MetaDataFetcher glyrFetcher_relations;
extern MetaDataFetcher glyrFetcher_tracklist;


/* Externalized sourceprovider vars, add yours here */
extern MetaDataSource ainfo_allmusic_src;
extern MetaDataSource ainfo_lastfm_src;
extern MetaDataSource ainfo_lyricsreg_src;
extern MetaDataSource albumlist_musicbrainz_src;
extern MetaDataSource cover_albumart_src;
extern MetaDataSource cover_allmusic_src;
extern MetaDataSource cover_amazon_src;
extern MetaDataSource cover_coverhunt_src;
extern MetaDataSource cover_discogs_src;
extern MetaDataSource cover_google_src;
extern MetaDataSource cover_lastfm_src;
extern MetaDataSource cover_lyricswiki_src;
extern MetaDataSource lyrics_lipwalk_src;
extern MetaDataSource lyrics_lyrdb_src;
extern MetaDataSource lyrics_lyricsreg_src;
extern MetaDataSource lyrics_lyricstime_src;
extern MetaDataSource lyrics_lyricsvip_src;
extern MetaDataSource lyrics_lyricswiki_src;
extern MetaDataSource lyrics_lyrix_src;
extern MetaDataSource lyrics_magistrix_src;
extern MetaDataSource lyrics_metrolyrics_src;
extern MetaDataSource photos_flickr_src;
extern MetaDataSource photos_google_src;
extern MetaDataSource photos_lastfm_src;
extern MetaDataSource relations_musicbrainz_src;
extern MetaDataSource review_allmusic_src;
extern MetaDataSource review_amazon_src;
extern MetaDataSource similar_artist_lastfm_src;
extern MetaDataSource similar_song_lastfm_src;
extern MetaDataSource tags_musicbrainz_src;
extern MetaDataSource tracklist_musicbrainz_src;

// Disabled due to bad quality.
//extern MetaDataSource lyrics_darklyrics_src;
//extern MetaDataSource lyrics_directlyrics_src;
//extern MetaDataSource lyrics_songlyrics_src;
/* --------------------------------------- */

/* Get fetcher list */
GList * r_getFList(void)
{
    return glyrMetaDataPluginList;
}
/* Get Source list */
GList * r_getSList(void)
{
    return glyrMetaDataSourceList;
}

/* --------------------------------------- */

static void register_provider_plugins(void)
{
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&ainfo_allmusic_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&ainfo_lastfm_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&ainfo_lyricsreg_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&albumlist_musicbrainz_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&cover_albumart_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&cover_allmusic_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&cover_amazon_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&cover_coverhunt_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&cover_discogs_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&cover_google_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&cover_lastfm_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&cover_lyricswiki_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&lyrics_darklyrics_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&lyrics_directlyrics_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&lyrics_lipwalk_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&lyrics_lyrdb_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&lyrics_lyricsreg_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&lyrics_lyricstime_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&lyrics_lyricsvip_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&lyrics_lyricswiki_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&lyrics_lyrix_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&lyrics_magistrix_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&lyrics_metrolyrics_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&lyrics_songlyrics_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&photos_flickr_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&photos_google_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&photos_lastfm_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&relations_musicbrainz_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&review_allmusic_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&review_amazon_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&similar_artist_lastfm_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&similar_song_lastfm_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&tags_musicbrainz_src);
    glyrMetaDataSourceList = g_list_prepend(glyrMetaDataSourceList,&tracklist_musicbrainz_src);
}

/* --------------------------------------- */

/* Just presort a bit, so we don't need to shuffle the plugins always *
 * This has no actual practical use, it's just that pretty plugins are listed
 * first in outputs. Actual sorting is not threadsafe! 
 */
static gint compare_by_priority(gconstpointer a, gconstpointer b)
{
    const MetaDataSource * sa = a;
    const MetaDataSource * sb = b;
    return (sa != NULL && sb != NULL) ? 
	   (sa->quality * sa->speed -
            sb->quality * sb->speed
           ) : 0;
}

/* --------------------------------------- */

static void get_list_from_type(MetaDataFetcher * fetch)
{
    GList * src;
    for(src = glyrMetaDataSourceList; src; src = src->next)
    {
        MetaDataSource * item = src->data;
        if(item && fetch->type == item->type)
        {
            fetch->provider = g_list_prepend(fetch->provider,item);
        }
    }
    fetch->provider = g_list_sort(fetch->provider,compare_by_priority);

}

/* --------------------------------------- */

static void init_provider_list(void)
{
    GList * fetch;
    for(fetch = glyrMetaDataPluginList; fetch; fetch = fetch->next)
        get_list_from_type((MetaDataFetcher *)(fetch->data));
}

/* --------------------------------------- */

/* Register fetchers */
void register_fetcher_plugins(void)
{
    /* add ypurs here */
    glyrMetaDataPluginList = g_list_prepend(glyrMetaDataPluginList,&glyrFetcher_cover);
    glyrMetaDataPluginList = g_list_prepend(glyrMetaDataPluginList,&glyrFetcher_lyrics);
    glyrMetaDataPluginList = g_list_prepend(glyrMetaDataPluginList,&glyrFetcher_artistphotos);
    glyrMetaDataPluginList = g_list_prepend(glyrMetaDataPluginList,&glyrFetcher_artistbio);
    glyrMetaDataPluginList = g_list_prepend(glyrMetaDataPluginList,&glyrFetcher_similiar_artists);
    glyrMetaDataPluginList = g_list_prepend(glyrMetaDataPluginList,&glyrFetcher_similar_song);
    glyrMetaDataPluginList = g_list_prepend(glyrMetaDataPluginList,&glyrFetcher_review);
    glyrMetaDataPluginList = g_list_prepend(glyrMetaDataPluginList,&glyrFetcher_albumlist);
    glyrMetaDataPluginList = g_list_prepend(glyrMetaDataPluginList,&glyrFetcher_tags);
    glyrMetaDataPluginList = g_list_prepend(glyrMetaDataPluginList,&glyrFetcher_relations);
    glyrMetaDataPluginList = g_list_prepend(glyrMetaDataPluginList,&glyrFetcher_tracklist);

    /* call init() */
    for(GList * elem = glyrMetaDataPluginList; elem; elem = elem->next)
    {
        MetaDataFetcher * fetch = elem->data;
        if(fetch->init != NULL)
        {
            fetch->init();
        }
    }

    /* le plugins */
    register_provider_plugins();

    init_provider_list();
}

/* --------------------------------------- */

/* Unregister 'em again */
void unregister_fetcher_plugins(void)
{
    /* Destroy all fetchers */
    if(glyrMetaDataPluginList)
    {
        GList * elem;
        for(elem = glyrMetaDataPluginList; elem != NULL; elem = elem->next)
        {
            MetaDataFetcher * item = elem->data;
            if(item->destroy != NULL)
            {
                item->destroy();
            }
            g_list_free(item->provider);
            item->provider = NULL;
        }
        g_list_free(glyrMetaDataPluginList);
        glyrMetaDataPluginList = NULL;
    }

    /* Also kill others */
    if(!glyrMetaDataSourceList)
    {
        g_list_free(glyrMetaDataSourceList);
        glyrMetaDataSourceList = NULL;
    }
}

/* --------------------------------------- */

GlyrFetcherInfo * get_plugin_info(void)
{
	GlyrFetcherInfo * head = NULL;
	GlyrFetcherInfo * prev_fetcher = NULL;
	for(GList * elem0 = r_getFList(); elem0; elem0 = elem0->next)
	{
		MetaDataFetcher * fetch = elem0->data;
		GlyrFetcherInfo  * finfo = g_malloc0(sizeof(GlyrFetcherInfo));

		GlyrSourceInfo * prev_source = NULL;
		for(GList * elem1 = r_getSList(); elem1; elem1 = elem1->next)
		{
			MetaDataSource * source = elem1->data;
			if(source && source->type == fetch->type)
			{
				GlyrSourceInfo  * sinfos = g_malloc0(sizeof(GlyrSourceInfo)); 

				sinfos->quality = source->quality;
				sinfos->speed   = source->speed;
				sinfos->key     = source->key;
				sinfos->type    = source->type;
				sinfos->name    = g_strdup(source->name);			

				if(prev_source != NULL)
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

		if(prev_fetcher != NULL)
		{
			prev_fetcher->next = finfo;
			finfo->prev = prev_fetcher;
		}
		else
		{
			head = finfo;
		}

		prev_fetcher = finfo;

		finfo->name = g_strdup(fetch->name);
		finfo->type = fetch->type;
	}
	return head;
}

/* --------------------------------------- */

static void free_single_item(GlyrFetcherInfo * info)
{
	if(info != NULL)
	{
		GlyrSourceInfo * elem = info->head;
		while(elem != NULL)
		{
			GlyrSourceInfo * to_delete = elem;
			elem = elem->next;

			g_free(to_delete->name);
			g_free(to_delete);
		}
		g_free((gchar*)info->name);
		info->name = NULL;
	}
	g_free(info);
}

/* --------------------------------------- */

void free_plugin_info(GlyrFetcherInfo ** infos)
{
	if(infos != NULL)
	{
		GlyrFetcherInfo * fetch = infos[0];
		while(fetch != NULL)
		{
			GlyrFetcherInfo * to_delete = fetch;
			fetch = fetch->next;

			free_single_item(to_delete);
		}
		*(infos) = NULL;
	}
}
