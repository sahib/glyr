/***********************************************************
 * This file is part of glyr
 * + a commnandline tool and library to download various sort of musicrelated metadata.
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

#include <glib.h>
#include <locale.h>

#include "glyr.h"
#include "core.h"
#include "register_plugins.h"
#include "blacklist.h"
#include "cache.h"
#include "stringlib.h"

//////////////////////////////////

static volatile gboolean is_initalized = FALSE;

//////////////////////////////////

static const char * err_strings[] =
{
    [GLYRE_UNKNOWN] = "Unknown error",
    [GLYRE_OK] = "No error",
    [GLYRE_BAD_VALUE] ="Bad value for glyr_opt_[...]()",
    [GLYRE_BAD_OPTION] = "Bad option passed to gly_opt_[...]()",
    [GLYRE_NO_PROVIDER] = "No valid provider specified in glyr_opt_from()",
    [GLYRE_EMPTY_STRUCT] = "Empty Query structure (NULL)",
    [GLYRE_UNKNOWN_GET] = "Unknown GLYR_GET_TYPE in glyr_get(); Use glyr_opt_type()",
    [GLYRE_INSUFF_DATA] = "Insufficient data supplied for this getter",
    [GLYRE_SKIP] = "Item was skipped due to user",
    [GLYRE_STOP_POST] = "Stopped by callback (POST)",
    [GLYRE_STOP_PRE] = "Stopped by callback (PRE)",
    [GLYRE_NO_INIT] = "Library is not yet initialized, use glyr_init()",
    [GLYRE_WAS_STOPPED] = "Library was stopped by glyr_signal_exit()"
};

static const char * type_strings[] =
{
    [GLYR_TYPE_COVERART] = "cover",
    [GLYR_TYPE_LYRICS] = "songtext",
    [GLYR_TYPE_ARTIST_PHOTO] = "artistphoto",
    [GLYR_TYPE_ALBUM_REVIEW] = "albumreview",
    [GLYR_TYPE_ARTIST_BIO] = "artistbio",
    [GLYR_TYPE_SIMILAR_ARTIST] = "similar_artist",
    [GLYR_TYPE_SIMILAR_SONG] = "similar_song",
    [GLYR_TYPE_TRACK] = "trackname",
    [GLYR_TYPE_ALBUMLIST] = "albumname",
    [GLYR_TYPE_TAG] = "tag",
    [GLYR_TYPE_TAG_ARTIST] = "artisttag",
    [GLYR_TYPE_TAG_ALBUM] = "albumtag",
    [GLYR_TYPE_TAG_TITLE] = "titletag",
    [GLYR_TYPE_RELATION] = "relation",
    [GLYR_TYPE_IMG_URL] = "ImageURL",
    [GLYR_TYPE_TXT_URL] = "HTMLURL",
    [GLYR_TYPE_GUITARTABS] = "guitartabs",
    [GLYR_TYPE_BACKDROPS] = "backdrop",
    [GLYR_TYPE_UNKNOWN] = "unknown"
};

/////////////////////////////////

const gchar * map_language[][2] =
{
    {"en_US","us"},
    {"en_CA","ca"},
    {"en_UK","uk"}
};


/////////////////////////////////

void glyr_internal_log (const gchar *log_domain,GLogLevelFlags log_level,const gchar *message, gpointer user_data)
{
    if (message != NULL) fputs (message,GLYR_OUTPUT);
}


/////////////////////////////////

/* Local scopes are cool. */
#define END_STRING(STR,CHAR)           \
    {                                      \
        gchar * term = strchr(STR,CHAR);   \
        if(term) *term = 0;                \
    }

/**
 * @brief Guesses the users language (in ISO639-1 codes like 'de') by the system locale
 *
 * @return a newly allocated language code. Free.
 */
gchar * guess_language (void)
{
    /* Default to 'en' in any case */
    gchar * result_lang = g_strdup ("en");

#if GLIB_CHECK_VERSION(2,28,0)
    gboolean break_out = FALSE;

    /* Please never ever free this */
    const gchar * const * languages = g_get_language_names();

    for (gint i = 0; languages[i] && break_out == FALSE; i++)
    {
        gchar ** variants = g_get_locale_variants (languages[i]);
        for (gint j = 0; variants[j] && !break_out; j++)
        {
            /* Look up if we need to map a language */
            gchar * to_investigate = variants[j];
            gint map_size = (sizeof (map_language) / (2 * sizeof (char*) ) );
            for (gint map = 0; map < map_size; map++)
            {
                const gchar * to_map = map_language[map][0];
                gsize map_len  = strlen (to_map);
                if (g_ascii_strncasecmp (to_map,to_investigate,map_len) == 0)
                {
                    to_investigate = (gchar*) map_language[map][1];
                    break;
                }
            }

            gboolean allowed_lang = TRUE;

            if (allowed_lang &&
                    g_ascii_strncasecmp ("en",to_investigate,2) != 0 &&
                    g_ascii_strncasecmp ("C", to_investigate,1) != 0 &&
                    !strchr (to_investigate,'@') && !strchr (to_investigate,'.') )
            {
                g_free (result_lang);
                result_lang = g_strdup (to_investigate);
                break_out = TRUE;
            }
        }
        g_strfreev (variants);
    }

#elif GLIB_CHECK_VERSION(2,26,0)

    /* Fallback to simpler version of the above,
     * g_get_locale_variants is not there in this version
     */
    const gchar * const * possible_locales = g_get_language_names();
    if (possible_locales != NULL)
    {
        /* might be a bit weird */
        for (gint i = 0; possible_locales[i]; i++)
        {
            if (g_ascii_strncasecmp ("en",possible_locales[i],2) != 0 &&
                    g_ascii_strncasecmp ("C", possible_locales[i],1) != 0)
            {
                g_free (result_lang);
                result_lang = g_strdup (possible_locales[i]);
                break;
            }
        }
    }
#else
    /* Fallback for version prior GLib 2.26:
     * Fallback to "en" always.
     * Gaaah... shame on you if this happens to you ;-)
     */
#endif

    /* Properly terminate string */
    END_STRING (result_lang,'_');
    END_STRING (result_lang,'@');
    END_STRING (result_lang,'.');

    /* We don't need it anymore */
#undef END_STRING

    return result_lang;
}

/////////////////////////////////

static int glyr_set_info (GlyrQuery * s, int at, const char * arg);
static void set_query_on_defaults (GlyrQuery * glyrs);

/////////////////////////////////
// OTHER
/////////////////////////////////

// return a descriptive string on error ID
__attribute__ ( (visibility ("default") ) )
const char * glyr_strerror (GLYR_ERROR ID)
{
    if (ID < (sizeof (err_strings) /sizeof (const char *) ) )
    {
        return err_strings[ID];
    }
    return err_strings[GLYRE_UNKNOWN];
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
void glyr_signal_exit (GlyrQuery * query)
{
    SET_ATOMIC_SIGNAL_EXIT (query,1);
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
void glyr_cache_update_md5sum (GlyrMemCache * cache)
{
    update_md5sum (cache);
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
void glyr_cache_set_data (GlyrMemCache * cache, const char * data, int len)
{
    DL_set_data (cache,data,len);
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GlyrMemCache * glyr_cache_copy (GlyrMemCache * cache)
{
    return DL_copy (cache);
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
const char * glyr_version (void)
{
    return "Version "GLYR_VERSION_MAJOR"."GLYR_VERSION_MINOR"."GLYR_VERSION_MICRO" ("GLYR_VERSION_NAME") of ["__DATE__"] compiled at ["__TIME__"]";
}

/////////////////////////////////
// _opt_
/////////////////////////////////

// Seperate method because va_arg struggles with function pointers
__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_dlcallback (GlyrQuery * settings, DL_callback dl_cb, void * userp)
{
    if (settings)
    {
        settings->callback.download     = dl_cb;
        settings->callback.user_pointer = userp;
        return GLYRE_OK;
    }
    return GLYRE_EMPTY_STRUCT;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_type (GlyrQuery * s, GLYR_GET_TYPE type)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    if (type != GLYR_GET_UNKNOWN)
    {
        s->type = type;
        return GLYRE_OK;
    }
    return GLYRE_BAD_VALUE;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_artist (GlyrQuery * s, const char * artist)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    glyr_set_info (s,0,artist);
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_album (GlyrQuery * s, const char * album)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    glyr_set_info (s,1,album);
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_title (GlyrQuery * s, const char * title)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    glyr_set_info (s,2,title);
    return GLYRE_OK;
}

/////////////////////////////////

static int size_set (int * ref, int size)
{
    if (size < -1 && ref)
    {
        *ref = -1;
        return GLYRE_BAD_VALUE;
    }

    if (ref)
    {
        *ref = size;
        return GLYRE_OK;
    }
    return GLYRE_BAD_OPTION;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_img_maxsize (GlyrQuery * s, int size)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    return size_set (&s->img_max_size,size);
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_img_minsize (GlyrQuery * s, int size)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    return size_set (&s->img_min_size,size);
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_parallel (GlyrQuery * s, unsigned long val)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    s->parallel = (long) val;
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_timeout (GlyrQuery * s, unsigned long val)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    s->timeout = (long) val;
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_redirects (GlyrQuery * s, unsigned long val)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    s->redirects = (long) val;
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_useragent (GlyrQuery * s, const char * useragent)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    glyr_set_info (s,6, (useragent) ? useragent : "");
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_lang (GlyrQuery * s, const char * langcode)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    if (langcode != NULL)
    {
        if (g_ascii_strncasecmp ("auto",langcode,4) == 0)
        {
            gchar * auto_lang = guess_language();
            glyr_set_info (s,7,auto_lang);
            g_free (auto_lang);
        }
        else
        {
            glyr_set_info (s,7,langcode);
        }
        return GLYRE_OK;
    }
    return GLYRE_BAD_VALUE;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_lang_aware_only (GlyrQuery * s, bool lang_aware_only)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    s->lang_aware_only = lang_aware_only;
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_number (GlyrQuery * s, unsigned int num)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    s->number = num == 0 ? INT_MAX : num;
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_verbosity (GlyrQuery * s, unsigned int level)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    s->verbosity = level;
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_from (GlyrQuery * s, const char * from)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    if (from != NULL)
    {
        glyr_set_info (s,4,from);
        return GLYRE_OK;
    }
    return GLYRE_BAD_VALUE;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_allowed_formats (GlyrQuery * s, const char * formats)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    glyr_set_info (s,5, (formats==NULL) ? GLYR_DEFAULT_ALLOWED_FORMATS : formats);
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_musictree_path (GlyrQuery * s, const char * musictree_path)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    glyr_set_info (s,8, (musictree_path==NULL) ? GLYR_DEFAULT_MUISCTREE_PATH : musictree_path);
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_plugmax (GlyrQuery * s, int plugmax)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    if (plugmax < 0)
    {
        return GLYRE_BAD_VALUE;
    }

    s->plugmax = plugmax;
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_qsratio (GlyrQuery * s, float ratio)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    s->qsratio = MIN (MAX (ratio,0.0),1.0);
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_proxy (GlyrQuery * s, const char * proxystring)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    glyr_set_info (s,3,proxystring);
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_fuzzyness (GlyrQuery * s, int fuzz)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    s->fuzzyness = fuzz;
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_download (GlyrQuery * s, bool download)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    s->download = download;
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_force_utf8 (GlyrQuery * s, bool force_utf8)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    s->force_utf8 = force_utf8;
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_lookup_db (GlyrQuery * s, GlyrDatabase * db)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    if (db != NULL)
    {
        s->local_db = db;
        return GLYRE_OK;
    }
    return GLYRE_BAD_VALUE;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_db_autowrite (GlyrQuery * s, bool db_autowrite)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    s->db_autowrite = db_autowrite;
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_db_autoread (GlyrQuery * s, bool db_autoread)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    s->db_autoread = db_autoread;
    return GLYRE_OK;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_ERROR glyr_opt_normalize (GlyrQuery * s, GLYR_NORMALIZATION norm)
{
    if (s == NULL) return GLYRE_EMPTY_STRUCT;
    s->normalization = norm;
    return GLYRE_OK;
}

/////////////////////////////////
/////////////////////////////////
/////////////////////////////////

static void set_query_on_defaults (GlyrQuery * glyrs)
{
    if (glyrs == NULL)
    {
        return;
    }

    /* Initialize free pointer pool */
    memset (glyrs,0,sizeof (GlyrQuery) );

    glyrs->type = GLYR_GET_UNKNOWN;
    glyrs->artist = NULL;
    glyrs->album  = NULL;
    glyrs->title  = NULL;
    glyrs->local_db = NULL;
    glyrs->callback.download = NULL;
    glyrs->callback.user_pointer = NULL;
    glyrs->musictree_path = NULL;
    glyrs->q_errno = GLYRE_OK;

    glyrs->db_autoread = GLYR_DEFAULT_DB_AUTOREAD;
    glyrs->db_autowrite = GLYR_DEFAULT_DB_AUTOWRITE;
    glyrs->from   = GLYR_DEFAULT_FROM;
    glyrs->img_min_size = GLYR_DEFAULT_CMINSIZE;
    glyrs->img_max_size = GLYR_DEFAULT_CMAXSIZE;
    glyrs->number = GLYR_DEFAULT_NUMBER;
    glyrs->parallel  = GLYR_DEFAULT_PARALLEL;
    glyrs->redirects = GLYR_DEFAULT_REDIRECTS;
    glyrs->timeout   = GLYR_DEFAULT_TIMEOUT;
    glyrs->verbosity = GLYR_DEFAULT_VERBOSITY;
    glyrs->plugmax = GLYR_DEFAULT_PLUGMAX;
    glyrs->download = GLYR_DEFAULT_DOWNLOAD;
    glyrs->fuzzyness = GLYR_DEFAULT_FUZZYNESS;
    glyrs->proxy = GLYR_DEFAULT_PROXY;
    glyrs->qsratio = GLYR_DEFAULT_QSRATIO;
    glyrs->allowed_formats = GLYR_DEFAULT_ALLOWED_FORMATS;
    glyrs->useragent = GLYR_DEFAULT_USERAGENT;
    glyrs->force_utf8 = GLYR_DEFAULT_FORCE_UTF8;
    glyrs->lang = GLYR_DEFAULT_LANG;
    glyrs->lang_aware_only = GLYR_DEFAULT_LANG_AWARE_ONLY;
    glyrs->normalization = GLYR_NORMALIZE_AGGRESSIVE | GLYR_NORMALIZE_ALL;
    glyrs->signal_exit = FALSE;
    glyrs->itemctr = 0;

    /* Set on a very specific value, so we can pretty sure,
       we are not accessing bad memory - feels little hackish.. */
    glyrs->is_initalized = QUERY_INITIALIZER;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
void glyr_query_init (GlyrQuery * glyrs)
{
    if (glyrs != NULL)
    {
        set_query_on_defaults (glyrs);
    }
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
void glyr_query_destroy (GlyrQuery * sets)
{
    if (sets != NULL && QUERY_IS_INITALIZED (sets) )
    {
        for (gsize i = 0; i < 10; i++)
        {
            if (sets->info[i] != NULL)
            {
                g_free ( (char*) sets->info[i]);
                sets->info[i] = NULL;
            }
        }

        /* Reset query so it can be used again */
        set_query_on_defaults (sets);
    }

}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GlyrMemCache * glyr_download (const char * url, GlyrQuery * s)
{
    return download_single (url,s,NULL);
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
void glyr_free_list (GlyrMemCache * head)
{
    if (head != NULL)
    {
        GlyrMemCache * next = head;
        GlyrMemCache * prev = head->prev;

        while (next != NULL)
        {
            GlyrMemCache * p = next;
            next = next->next;
            DL_free (p);
        }

        while (prev != NULL)
        {
            GlyrMemCache * p = prev;
            prev = prev->prev;
            DL_free (p);
        }
    }
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
void glyr_cache_free (GlyrMemCache * c)
{
    DL_free (c);
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GlyrMemCache * glyr_cache_new (void)
{
    return DL_init();
}

/////////////////////////////////

// !! NOT THREADSAFE !! //
__attribute__ ( (visibility ("default") ) )
void glyr_init (void)
{
    /* Protect agains double initialization */
    if (is_initalized == FALSE)
    {
        /* Set loghandler */
        g_log_set_handler (G_LOG_DOMAIN, G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL
                           | G_LOG_FLAG_RECURSION,
                           glyr_internal_log, NULL);

        if (curl_global_init (CURL_GLOBAL_ALL) )
        {
            glyr_message (-1,NULL,"Fatal: libcurl failed to init\n");
        }

        /* Locale */
        if (setlocale (LC_ALL, "") == NULL)
        {
            glyr_message (-1,NULL,"Cannot set locale!\n");
        }

        /* Register plugins */
        register_fetcher_plugins();

        /* Init the smallest blacklist in the world :-) */
        blacklist_build();

        is_initalized = TRUE;
    }
}


/////////////////////////////////

// !! NOT THREADSAFE !! //
__attribute__ ( (visibility ("default") ) )
void glyr_cleanup (void)
{
    if (is_initalized == TRUE)
    {
        /* Curl no longer needed */
        curl_global_cleanup();

        /* Destroy all fetchers */
        unregister_fetcher_plugins();

        /* Kill it again */
        blacklist_destroy();

        is_initalized = FALSE;
    }
}



/////////////////////////////////

/* Sets parallel field depending on the get_type */
static void auto_detect_parallel (MetaDataFetcher * fetcher, GlyrQuery * query)
{
    if (query->parallel <= 0)
    {
        if (fetcher->default_parallel <= 0)
        {
            gint div = (int) (1.0/ (CLAMP (query->qsratio,0.01,0.99) * 2) );
            query->parallel = (div == 0) ? 3 : g_list_length (fetcher->provider) / div;
        }
        else
        {
            query->parallel = fetcher->default_parallel;
        }
    }
}

/////////////////////////////////

static gboolean check_if_valid (GlyrQuery * q, MetaDataFetcher * fetch)
{
    gboolean isValid = TRUE;
    if (fetch->reqs & GLYR_REQUIRES_ARTIST && q->artist == NULL)
    {
        glyr_message (2,q,"Artist is required for this getter\n");
        isValid = FALSE;
    }

    if (fetch->reqs & GLYR_REQUIRES_ALBUM && q->album == NULL)
    {
        glyr_message (2,q,"Albumname is required for this getter\n");
        isValid = FALSE;
    }

    if (fetch->reqs & GLYR_REQUIRES_TITLE && q->title == NULL)
    {
        glyr_message (2,q,"Songname is required for this getter\n");
        isValid = FALSE;
    }

    if (isValid == FALSE && fetch->reqs > GLYR_REQUIRES_TITLE)
    {
        char * f1 = "",* f2 = "", * f3 = "";
        if (fetch->reqs & GLYR_OPTIONAL_ARTIST) f1 = "Artist ";
        if (fetch->reqs & GLYR_OPTIONAL_ALBUM)  f2 = "Album ";
        if (fetch->reqs & GLYR_OPTIONAL_TITLE)  f3 = "Songtitle ";

        if (*f1 || *f2 || *f3)
        {
            glyr_message (2,q,"\nFollowing fields are optional: %s%s%s\n",f1,f2,f3);
        }
    }
    return isValid;
}

/////////////////////////////////

static void set_error (GLYR_ERROR err_in, GlyrQuery * query, GLYR_ERROR * err_out)
{
    if (query)
        query->q_errno = err_in;
    if (err_out)
        *err_out = err_in;
}

/////////////////////////////////


#define PRINT_NORMALIZED_ATTR(name, mode, var)              \
    if(var != NULL && query->verbosity >= 2)                \
    {                                                       \
        char * prepared = prepare_string(var, mode, FALSE); \
        if(prepared != NULL)                                \
        {                                                   \
            glyr_message(2, query, name);                   \
            glyr_message(2, query, "%s\n", prepared);       \
            g_free(prepared);                               \
        }                                                   \
    }                                                       \

__attribute__ ( (visibility ("default") ) )
GlyrMemCache * glyr_get (GlyrQuery * query, GLYR_ERROR * e, int * length)
{
    if (is_initalized == FALSE || QUERY_IS_INITALIZED (query) == FALSE)
    {
        glyr_message (-1,NULL,"Warning: Either query or library is not initialized.\n");
        if (e != NULL)
        {
            set_error (GLYRE_NO_INIT, query, e);
        }
        return NULL;
    }

    set_error (GLYRE_OK, query, e);

    if (query != NULL)
    {
        if (g_ascii_strncasecmp (query->lang,"auto",4) == 0)
        {
            glyr_opt_lang (query,"auto");
        }

        GList * result = NULL;
        set_error (GLYRE_UNKNOWN_GET, query, e);

        for (GList * elem = r_getFList(); elem; elem = elem->next)
        {
            MetaDataFetcher * item = elem->data;
            if (query->type == item->type)
            {
                if (check_if_valid (query,item) == TRUE)
                {
                    /* Print some user info, always useful */
                    if (query->normalization & GLYR_NORMALIZE_ARTIST) {
                        PRINT_NORMALIZED_ATTR("- Artist   : ", query->normalization, query->artist);
                    } else {
                        PRINT_NORMALIZED_ATTR("- Artist   : ", GLYR_NORMALIZE_NONE, query->artist);
                    }

                    if (query->normalization & GLYR_NORMALIZE_ALBUM) {
                        PRINT_NORMALIZED_ATTR("- Album    : ", query->normalization, query->album);
                    } else {
                        PRINT_NORMALIZED_ATTR("- Album    : ", GLYR_NORMALIZE_NONE, query->album);
                    }

                    if (query->normalization & GLYR_NORMALIZE_TITLE) {
                        PRINT_NORMALIZED_ATTR("- Title    : ", query->normalization, query->title);
                    } else {
                        PRINT_NORMALIZED_ATTR("- Title    : ", GLYR_NORMALIZE_NONE, query->title);
                    }

                    if (query->lang != NULL)
                    {
                        glyr_message (2,query,"- Language : ");
                        glyr_message (2,query,"%s\n",query->lang);
                    }

                    set_error (GLYRE_OK, query, e);
                    glyr_message (2,query,"- Type     : %s\n\n",item->name);

                    /* Lookup what we search for here: Images (url, or raw) or text */
                    query->imagejob = ! (item->full_data);

                    /* If ->parallel is <= 0, it gets autodetected */
                    auto_detect_parallel (item, query);

                    /* Now start your engines, gentlemen */
                    result = start_engine (query,item,e);
                    break;
                }
                else
                {
                    set_error (GLYRE_INSUFF_DATA, query, e);
                }
            }
        }

        /* Make this query reusable */
        query->itemctr = 0;

        /* Start of the returned list */
        GlyrMemCache * head = NULL;

        /* Librarby was stopped, just return NULL. */
        if (result != NULL && GET_ATOMIC_SIGNAL_EXIT (query) )
        {
            for (GList * elem = result; elem; elem = elem->next)
                DL_free (elem->data);

            g_list_free (result);
            result = NULL;

            set_error (GLYRE_WAS_STOPPED, query, e);
        }

        /* Set the length */
        if (length != NULL)
        {
            *length = g_list_length (result);
        }

        /* free if empty */
        if (result != NULL)
        {
            /* Count inserstions */
            gint db_inserts = 0;

            /* link caches to each other */
            for (GList * elem = result; elem; elem = elem->next)
            {
                GlyrMemCache * item = elem->data;
                item->next = (elem->next) ? elem->next->data : NULL;
                item->prev = (elem->prev) ? elem->prev->data : NULL;

                if (query->db_autowrite && query->local_db && item->cached == FALSE)
                {
                    db_inserts++;
                    glyr_db_insert (query->local_db,query,item);
                }
            }

            if (db_inserts > 0)
            {
                glyr_message (2,query,"--- Inserted %d item%s into db.\n",db_inserts, (db_inserts == 1) ? "" : "s");
            }

            /* Finish. */
            if (g_list_first (result) )
            {
                head = g_list_first (result)->data;
            }

            g_list_free (result);
            result = NULL;
        }

        /* Done! */
        SET_ATOMIC_SIGNAL_EXIT (query,0);
        return head;
    }

    set_error (GLYRE_EMPTY_STRUCT, query, e);
    SET_ATOMIC_SIGNAL_EXIT (query,0);
    return NULL;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
int glyr_cache_write (GlyrMemCache * data, const char * path)
{
    int bytes = -1;
    if (path)
    {
        if (!g_ascii_strcasecmp (path,"null") )
        {
            bytes = 0;
        }
        else if (!g_ascii_strcasecmp (path,"stdout") )
        {
            bytes=fwrite (data->data,1,data->size,stdout);
            fputc ('\n',stdout);
        }
        else if (!g_ascii_strcasecmp (path,"stderr") )
        {
            bytes=fwrite (data->data,1,data->size,stderr);
            fputc ('\n',stderr);
        }
        else
        {
            FILE * fp = fopen (path,"w");
            if (fp)
            {
                if (data->data != NULL)
                {
                    bytes=fwrite (data->data,1,data->size,fp);
                }
                fclose (fp);
            }
            else
            {
                glyr_message (-1,NULL,"glyr_cache_write: Unable to write to '%s'!\n",path);
            }
        }
    }
    return bytes;
}

/////////////////////////////////

static int glyr_set_info (GlyrQuery * s, int at, const char * arg)
{
    gint result = GLYRE_OK;
    if (s && arg && at >= 0 && at < 10)
    {
        if (s->info[at] != NULL)
        {
            g_free ( (char*) s->info[at]);
        }

        s->info[at] = g_strdup (arg);
        switch (at)
        {
        case 0:
            s->artist = (gchar*) s->info[at];
            break;
        case 1:
            s->album = (gchar*) s->info[at];
            break;
        case 2:
            s->title = (gchar*) s->info[at];
            break;
        case 3:
            s->proxy = s->info[at];
            break;
        case 4:
            s->from = (gchar*) s->info[at];
            break;
        case 5:
            s->allowed_formats = (gchar*) s->info[at];
            break;
        case 6:
            s->useragent = (gchar*) s->info[at];
            break;
        case 7:
            s->lang = (gchar*) s->info[at];
            break;
        case 8:
            s->musictree_path = (gchar * ) s->info[at];
            break;
        default:
            glyr_message (2,s,"Warning: wrong <at> for glyr_info_at!\n");
        }
    }
    else
    {
        result = GLYRE_BAD_VALUE;
    }
    return result;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
const char * glyr_get_type_to_string (GLYR_GET_TYPE type)
{
    const gchar * result = "unknown";

    GList * fetcher_list = r_getFList();
    for (GList * elem = fetcher_list; elem != NULL; elem = elem->next)
    {
        MetaDataFetcher * fetch = elem->data;
        if (fetch->type == type)
        {
            result = fetch->name;
            break;
        }
    }

    return result;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_GET_TYPE glyr_string_to_get_type (const char * string)
{
    if (string != NULL)
    {
        GList * fetcher_list = r_getFList();
        for (GList * elem = fetcher_list; elem != NULL; elem = elem->next)
        {
            MetaDataFetcher * fetch = elem->data;
            if (g_ascii_strcasecmp (fetch->name,string) == 0)
            {
                return fetch->type;
                break;
            }
        }
    }
    return GLYR_GET_UNKNOWN;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
const char * glyr_data_type_to_string (GLYR_DATA_TYPE type)
{
    if (type > 0 && type < (sizeof (type_strings) /sizeof (const char*) ) )
    {
        return type_strings[type];
    }
    else
    {
        return type_strings[GLYR_TYPE_UNKNOWN];
    }
}
/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_DATA_TYPE glyr_string_to_data_type (const char * string)
{
    if (string != NULL)
    {
        gsize table_size = (sizeof (type_strings) /sizeof (const char*) );
        for (gsize i = 0; i < table_size; i++)
        {
            if (g_ascii_strcasecmp (string,type_strings[i]) == 0)
                return i;
        }
    }
    return GLYR_TYPE_UNKNOWN;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
void glyr_cache_print (GlyrMemCache * cacheditem)
{
    if (cacheditem != NULL)
    {
        glyr_message (-1,NULL,"FROM: <%s>\n",cacheditem->dsrc);
        glyr_message (-1,NULL,"PROV: %s\n",cacheditem->prov);
        glyr_message (-1,NULL,"SIZE: %d Bytes\n", (int) cacheditem->size);
        glyr_message (-1,NULL,"MSUM: ");


        /* Print md5sum */
        for (int i = 0; i < 16; i++)
        {
            fprintf (stderr,"%02x", cacheditem->md5sum[i]);
        }

        // Each cache identified it's data by a constant
        glyr_message (-1,NULL,"\nTYPE: ");
        if (cacheditem->type == GLYR_TYPE_TRACK)
        {
            glyr_message (-1,NULL,"[%02d:%02d] ",cacheditem->duration/60, cacheditem->duration%60);
        }
        glyr_message (-1,NULL,"%s",glyr_data_type_to_string (cacheditem->type) );

        glyr_message (-1,NULL,"\nSAFE: %s", (cacheditem->cached) ? "Yes" : "No");
        glyr_message (-1,NULL,"\nRATE: %d",cacheditem->rating);
        glyr_message (-1,NULL,"\nSTMP: %f",cacheditem->timestamp);

        /* Print the actual data.
         * This might have funny results if using cover/photos
         */
        if (cacheditem->is_image == FALSE)
        {
            glyr_message (-1,NULL,"\nDATA: \n%s",cacheditem->data);
        }
        else
        {
            glyr_message (-1,NULL,"\nFRMT: %s",cacheditem->img_format);
            glyr_message (-1,NULL,"\nDATA: <not printable>");
        }
        glyr_message (-1,NULL,"\n");
    }
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GlyrFetcherInfo * glyr_info_get (void)
{
    return get_plugin_info();
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
void glyr_info_free (GlyrFetcherInfo * info)
{
    free_plugin_info (info);
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
char * glyr_md5sum_to_string (unsigned char * md5sum)
{
    gchar * md5str = NULL;
    if (md5sum != NULL)
    {
        const gchar * hex = "0123456789abcdef";
        md5str = g_malloc0 (33);
        for (int i = 0; i < 16; i++)
        {
            gint index = i * 2;
            md5str[index + 0] = hex[md5sum[i] / 16];
            md5str[index + 1] = hex[md5sum[i] % 16];
        }
    }
    return md5str;
}

/////////////////////////////////

#define CHAR_TO_NUM(c) (unsigned char)(g_ascii_isdigit(c) ? c - '0' : (c - 'a') + 10)

__attribute__ ( (visibility ("default") ) )
void glyr_string_to_md5sum (const char * string, unsigned char * md5sum)
{
    if (string != NULL && strlen (string) >= 32 && md5sum)
    {
        for (gint i = 0; i < 16; i++)
        {
            gint index = i * 2;
            md5sum[i] = (CHAR_TO_NUM (string[index]) << 4) + CHAR_TO_NUM (string[index+1]);
        }
    }
}
#undef CHAR_TO_NUM

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
GLYR_FIELD_REQUIREMENT glyr_get_requirements (GLYR_GET_TYPE type)
{
    GLYR_FIELD_REQUIREMENT result = 0;
    GlyrFetcherInfo * info = glyr_info_get();
    GlyrFetcherInfo * head = info;
    while (head != NULL)
    {
        if (type == head->type)
        {
            result = head->reqs;
        }
        head = head->next;
    }
    glyr_info_free (info);
    return result;
}

/* ---------------------------------------------------------
 * Setters for MemCache
 * There is not a setter for everything, just for things,
 * you might be interested in to set.
 * (No setter for is_image e.g.)
 * ---------------------------------------------------------
 */

#define SET_CACHE_STRING(Cache, Field, Value) \
    if(Cache != NULL) {                       \
        if(Field != NULL) {                   \
            g_free(Field);                    \
        }                                     \
        Field = g_strdup(Value);              \
    }

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
void glyr_cache_set_dsrc (GlyrMemCache * cache, const char * download_source)
{
    SET_CACHE_STRING (cache,cache->dsrc,download_source);
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
void glyr_cache_set_prov (GlyrMemCache * cache, const char * provider)
{
    SET_CACHE_STRING (cache,cache->prov,provider);
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
void glyr_cache_set_img_format (GlyrMemCache * cache, const char * img_format)
{
    SET_CACHE_STRING (cache,cache->img_format,img_format);
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
void glyr_cache_set_type (GlyrMemCache * cache, GLYR_DATA_TYPE type)
{
    if (cache != NULL)
        cache->type = MAX (type,GLYR_TYPE_UNKNOWN);
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
void glyr_cache_set_rating (GlyrMemCache * cache, int rating)
{
    if (cache != NULL)
        cache->rating = rating;
}

/////////////////////////////////

__attribute__ ( (visibility ("default") ) )
bool glyr_type_is_image (GLYR_GET_TYPE type)
{
    return TYPE_IS_IMAGE (type);
}

/////////////////////////////////
