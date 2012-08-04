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
#ifndef TYPES_H
#define TYPES_H

/**
 * SECTION:types
 * @short_description: Types, Enums and Defines of libglyr
 * @title: Types
 * @section_id:
 * @stability: Stable
 * @include: glyr/types.h
 *
 * All structs / enums / defines of the libglyr API can be found here.
 * You do not need to include this header directly, use glyr/glyr.h
 */
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <sqlite3.h>

    /* Versioninfo */
#include "config.h"

    /* Default values */
#define GLYR_DEFAULT_REDIRECTS 3L
#define GLYR_DEFAULT_TIMEOUT  20L
#define GLYR_DEFAULT_PARALLEL 0L
#define GLYR_DEFAULT_CMINSIZE 130
#define GLYR_DEFAULT_CMAXSIZE -1
#define GLYR_DEFAULT_VERBOSITY 0
#define GLYR_DEFAULT_NUMBER 1
#define GLYR_DEFAULT_PLUGMAX -1
#define GLYR_DEFAULT_LANG "auto"
#define GLYR_DEFAULT_DOWNLOAD true
#define GLYR_DEFAULT_FROM "all"
#define GLYR_DEFAULT_FROM_ARGUMENT_DELIM ";"
#define GLYR_DEFAULT_FUZZYNESS 4
#define GLYR_DEFAULT_PROXY NULL
#define GLYR_DEFAULT_QSRATIO 0.85
#define GLYR_DEFAULT_FORCE_UTF8 false
#define GLYR_DEFAULT_DB_AUTOWRITE true
#define GLYR_DEFAULT_DB_AUTOREAD true
#define GLYR_DEFAULT_MUISCTREE_PATH NULL
#define GLYR_DEFAULT_SUPPORTED_LANGS "en;de;fr;es;it;jp;pl;pt;ru;sv;tr;zh"
#define GLYR_DEFAULT_LANG_AWARE_ONLY false
#define GLYR_DEFAULT_NORMALIZATION GLYR_NORMALIZE_MODERATE

    /* Disallow *.gif, mostly bad quality
     * jpeg and jpg, because some not standardaware
     * servers give MIME types like image/jpg
     * (which should be image/jpeg usually)
     */
#define GLYR_DEFAULT_ALLOWED_FORMATS "png;jpeg;tiff;jpg;"

    /* Be honest by default */
#define GLYR_DEFAULT_USERAGENT "libglyr/"GLYR_VERSION_MAJOR"."GLYR_VERSION_MINOR"-"GLYR_VERSION_MICRO" ("GLYR_VERSION_NAME") +https://www.github.com/sahib/glyr"

    /* --------------------------- */
    /* --------- GROUPS ---------- */
    /* --------------------------- */

    /**
     * GLYR_NORMALIZATION:
     * @GLYR_NORMALIZE_NONE: Do no normalization to input artist/album/title.
     * @GLYR_NORMALIZE_MODERATE: Do quite some normalization, but don't break stuff.
     * @GLYR_NORMALIZE_AGGRESSIVE: Do everything under MODERATE, but also remove everythin in () [] and <>.
     * @GLYR_NORMALIZE_ARTIST: Normalize artist field, when set.
     * @GLYR_NORMALIZE_ALBUM: Normalize album field, when set.
     * @GLYR_NORMALIZE_TITLE: Normalize title field, when set.
     * @GLYR_NORMALIZE_ALL: Normalize all fields, when set.
     *
     * The normalization levels, that may be applied to artist/album/title.
     *
     * Default is: GLYR_NORMALIZE_AGGRESSIVE | GLYR_NORMALIZE_ALL
     **/
    typedef enum
    {
        GLYR_NORMALIZE_NONE       =  1 << 0,
        GLYR_NORMALIZE_MODERATE   =  1 << 1,
        GLYR_NORMALIZE_AGGRESSIVE =  1 << 2,
        GLYR_NORMALIZE_ARTIST     =  1 << 3,
        GLYR_NORMALIZE_ALBUM      =  1 << 4,
        GLYR_NORMALIZE_TITLE      =  1 << 5,
        GLYR_NORMALIZE_ALL        =  GLYR_NORMALIZE_ARTIST | GLYR_NORMALIZE_ALBUM | GLYR_NORMALIZE_TITLE
    }
                                 GLYR_NORMALIZATION;

    /**
     * GLYR_ERROR:
     * @GLYRE_UNKNOWN: Unknown error
     * @GLYRE_OK: everything is fine
     * @GLYRE_BAD_OPTION: you passed a bad option to glyr_opt_*
     * @GLYRE_BAD_VALUE: Invalid value in glyr_opt_*
     * @GLYRE_EMPTY_STRUCT: you passed an empty struct to glyr_opt_
     * @GLYRE_NO_PROVIDER: No valid provider specified in glyr_opt_from()
     * @GLYRE_UNKNOWN_GET: settings->type is not valid
     * @GLYRE_INSUFF_DATA: Insufficient data supplied; (artist/album/title) missing
     * @GLYRE_SKIP: If returned by callback, cache wont be added to results
     * @GLYRE_STOP_POST: Will stop searching, but still add the current item
     * @GLYRE_STOP_PRE: Will stop searching, but won't add the current item
     * @GLYRE_NO_INIT: Library has not been initialized with glyr_init() yet
     * @GLYRE_WAS_STOPPED: Library was stopped by glyr_signal_exit()
     *
     * All errors you can get, via glyr_get() and the glyr_opt_* calls.
     *
     * Use glyr_strerror() to convert them to meaningful strings.
     *
     **/
    typedef enum
    {
        GLYRE_UNKNOWN = 0,
        GLYRE_OK,
        GLYRE_BAD_OPTION,
        GLYRE_BAD_VALUE,
        GLYRE_EMPTY_STRUCT,
        GLYRE_NO_PROVIDER,
        GLYRE_UNKNOWN_GET,
        GLYRE_INSUFF_DATA,
        GLYRE_SKIP,
        GLYRE_STOP_POST,
        GLYRE_STOP_PRE,
        GLYRE_NO_INIT,
        GLYRE_WAS_STOPPED
    }
    GLYR_ERROR;

    /**
    * GLYR_GET_TYPE:
    * @GLYR_GET_COVERART: Get Coverart.
    * @GLYR_GET_LYRICS:  Get Songtext
    * @GLYR_GET_ARTIST_PHOTOS: Get Live/Promotion/Fanartwork
    * @GLYR_GET_ARTIST_BIO: Get information about this artist
    * @GLYR_GET_SIMILAR_ARTISTS: get similar artists
    * @GLYR_GET_SIMILAR_SONGS: get similar songs
    * @GLYR_GET_ALBUM_REVIEW: get album reviews
    * @GLYR_GET_TRACKLIST: get a tracklist for a specified album
    * @GLYR_GET_TAGS: get tags for a certain item
    * @GLYR_GET_RELATIONS: get relations for a certain item
    * @GLYR_GET_ALBUMLIST: get a list of albums from a certain artist
    * @GLYR_GET_GUITARTABS: get guitar tabs for a specified artist/title
    * @GLYR_GET_BACKDROPS: get large backdrops (backgrounds) from a certain artist.
    * @GLYR_GET_UNKNOWN: The default value after initializing a query.
    * @GLYR_GET_ANY: Delievers everything (like the 'locale' provider), do not use this.
    *
    * You tell libglyr what metadata you want by choosing one of the below
    * and set it via glyr_opt_type()
    *
    */

    /* DO NOT CHANGE THE ORDER HERE
     * The number there is saved in the cache to save it's type.
     * You can safely append elements though, before GLYR_GET_ANY.
     */
    typedef enum
    {
        GLYR_GET_UNKNOWN,
        GLYR_GET_COVERART,
        GLYR_GET_LYRICS,
        GLYR_GET_ARTIST_PHOTOS,
        GLYR_GET_ARTIST_BIO,
        GLYR_GET_SIMILAR_ARTISTS,
        GLYR_GET_SIMILAR_SONGS,
        GLYR_GET_ALBUM_REVIEW,
        GLYR_GET_TRACKLIST,
        GLYR_GET_TAGS,
        GLYR_GET_RELATIONS,
        GLYR_GET_ALBUMLIST,
        GLYR_GET_GUITARTABS,
        GLYR_GET_BACKDROPS,
        GLYR_GET_ANY
    }   GLYR_GET_TYPE;

    /**
    * GLYR_DATA_TYPE:
    * @GLYR_TYPE_UNKNOWN: You shouldn't get this
    * @GLYR_TYPE_LYRICS: Songtext
    * @GLYR_TYPE_ALBUM_REVIEW: Albumreview
    * @GLYR_TYPE_ARTIST_PHOTO: Pictures showing a certain band
    * @GLYR_TYPE_COVERART: coverart
    * @GLYR_TYPE_ARTIST_BIO: Artist bio
    * @GLYR_TYPE_SIMILAR_ARTIST: Similiar artists
    * @GLYR_TYPE_SIMILAR_SONG: Similar songs
    * @GLYR_TYPE_ALBUMLIST: List of albums: each cache containing one name
    * @GLYR_TYPE_TAG: List of (random) tags: each cache containing one name
    * @GLYR_TYPE_TAG_ARTIST: Tag associated with the artist
    * @GLYR_TYPE_TAG_ALBUM: Tag associated with the album
    * @GLYR_TYPE_TAG_TITLE: Tag associated with the album
    * @GLYR_TYPE_RELATION: Random relation: each cache containing one link
    * @GLYR_TYPE_IMG_URL: URL pointing to an image
    * @GLYR_TYPE_TXT_URL: URL pointing to some text content
    * @GLYR_TYPE_TRACK: List of tracknames: each cache containing one name
    * @GLYR_TYPE_GUITARTABS: Guitartabs
    * @GLYR_TYPE_BACKDROPS: Wallpaper-like artist images
    *
    * Mainly used in the 'type' field of GlyrMemCache.
    * It describes what kind of data the cache holds.
    * As a user of the API you will need this very seldom.
    * libglyr makes internally use of it.
    */

    /* DO NOT CHANGE THE ORDER HERE
     * The number there is saved in the cache to save it's type.
     * You can safely append elements though.
     */
    typedef enum
    {
        GLYR_TYPE_UNKNOWN,
        GLYR_TYPE_LYRICS,
        GLYR_TYPE_ALBUM_REVIEW,
        GLYR_TYPE_ARTIST_PHOTO,
        GLYR_TYPE_COVERART,
        GLYR_TYPE_ARTIST_BIO,
        GLYR_TYPE_SIMILAR_ARTIST,
        GLYR_TYPE_SIMILAR_SONG,
        GLYR_TYPE_ALBUMLIST,
        GLYR_TYPE_TAG,
        GLYR_TYPE_TAG_ARTIST,
        GLYR_TYPE_TAG_ALBUM,
        GLYR_TYPE_TAG_TITLE,
        GLYR_TYPE_RELATION,
        GLYR_TYPE_IMG_URL,
        GLYR_TYPE_TXT_URL,
        GLYR_TYPE_TRACK,
        GLYR_TYPE_GUITARTABS,
        GLYR_TYPE_BACKDROPS
    }   GLYR_DATA_TYPE;


    /* Silly compatibility defines to mantain backwards compatibility */
#define   GLYR_TYPE_SIMILIAR_ARTIST   GLYR_TYPE_SIMILAR_ARTIST
#define   GLYR_TYPE_SIMILIAR_SONG     GLYR_TYPE_SIMILAR_SONG
#define   GLYR_GET_SIMILIAR_ARTISTS   GLYR_GET_SIMILAR_ARTISTS
#define   GLYR_GET_SIMILIAR_SONGS     GLYR_GET_SIMILAR_SONGS
#define   GLYR_GET_ARTISTBIO          GLYR_GET_ARTIST_BIO
#define   GLYR_GET_UNSURE             GLYR_GET_UNKNOWN
#define   GLYR_TYPE_NOIDEA            GLYR_TYPE_UNKNOWN

    /**
    * GLYR_FIELD_REQUIREMENT:
    * @GLYR_REQUIRES_ARTIST: This getter needs the artist field
    * @GLYR_REQUIRES_ALBUM:  This getter needs the album field
    * @GLYR_REQUIRES_TITLE:  This getter needs the title field
    * @GLYR_OPTIONAL_ARTIST: Artist is optional for this getter
    * @GLYR_OPTIONAL_ALBUM:  Album is optional for this getter
    * @GLYR_OPTIONAL_TITLE:  Title is optional for this getter
    *
    * Bitmasks you can use to determine what fields a certain getter needs.
    * You can obtain it in the 'reqs' field of GlyrFetcherInfo (retrieved via glyr_info_get())
    */
    typedef enum
    {
        GLYR_REQUIRES_ARTIST = 1 << 0,
        GLYR_REQUIRES_ALBUM  = 1 << 1,
        GLYR_REQUIRES_TITLE  = 1 << 2,
        GLYR_OPTIONAL_ARTIST = 1 << 3,
        GLYR_OPTIONAL_ALBUM  = 1 << 4,
        GLYR_OPTIONAL_TITLE  = 1 << 5
    }   GLYR_FIELD_REQUIREMENT;

    /**
     * GlyrMemCache:
     * @data: contains the data, string when is_image is false, raw data otherwise
     * @size: Size of this item in bytes
     * @dsrc: URL pointing to the origin of this item.
     * @prov: name of the provider that delivered this item.
     * @type: The #GLYR_DATA_TYPE of this item.
     * @duration: For tracklist, only. Contains the tracklength in seconds.
     * @rating: Always set to 0, you can set this to rate this item. For use in the Database.
     * @is_image: Is this item an image?
     * @img_format: Format of the image (png,jpeg), NULL if text item.
     * @md5sum: A md5sum of the data field.
     * @cached: If this cache was locally cached.
     * @timestamp: This is used internally by libglyr.
     * @next: A pointer to the next item in the list, or NULL
     * @prev: A pointer to the previous item in the list, or NULL
     *
     * GlyrMemCache represents a single item received by libglyr.
     * You should <emphasis>NOT</emphasis> modify any of the fields directly, they are meant to be read-only.
     * If you need to set any field (usually only necessary in conjunction with glyr/cache.h) you may
     * want to use the glyr_cache_set_[public field] routines to safely modify the data.
     */
    typedef struct _GlyrMemCache
    {

        /*< public >*/
        char  * data;
        size_t size;
        char  * dsrc;
        char * prov;
        GLYR_DATA_TYPE type;
        int  duration;
        int  rating;
        bool is_image;
        char * img_format;
        unsigned char md5sum[16];
        bool cached;
        double timestamp;

        struct _GlyrMemCache * next;
        struct _GlyrMemCache * prev;
    } GlyrMemCache;

    /**
     * GlyrDatabase:
     * @root_path: The directory where the database will be stored.
     *
     * Represents an opaque database structure where caches may be fetched from.
     * It's members should not be accessed directly.
     */
    struct _GlyrDatabase;
    typedef struct _GlyrDatabase
    {
        /*< public >*/
        char * root_path;

        /*< private >*/
        sqlite3 * db_handle;

    } GlyrDatabase;

    /**
    * GlyrQuery:
    * @type: The type of metadata to get.
    * @number: The maximum number of items to get; 0 -> inf
    * @plugmax: Max number of items per provider.
    * @verbosity: How verbose this query should be treated.
    * @fuzzyness: Max. threshold for levenshtein.
    * @img_min_size: Min. size in pixels an image may have.
    * @img_max_size: Min. size in pixels an image may have.
    * @parallel: Max. number of parallel queried providers.
    * @timeout: Max. timeout in seconds to wait before cancelling a download.
    * @redirects: Max number of redirects. You shouldn't set this.
    * @force_utf8: Should be UTF8 forced on text items?
    * @download: should be images downloaded?
    * @qsratio: 0.0 = maxspeed, 1.0 = max quality, 0.85 -> default.
    * @db_autoread: Check if the found item is already cached.
    * @db_autowrite: Write found items automagically to the cache, if any specified by glyr_opt_lookup_db()
    * @local_db: The database to write and search in.
    * @lang_aware_only: Use only providers that deliver language specific content.
    * @signal_exit: By default false, but true when stopping searching is required.
    * @lang: Language code ISO-639-1, like 'de','en' or 'auto'
    * @proxy: The proxy to use.
    * @artist: Artist to use.
    * @album: Album to use.
    * @title: Title to use.
    * @from: Define what providers are queried.
    * @allowed_formats: Allowed imageformats.
    * @useragent: Useragent to use during http-requests
    * @musictree_path: Used for the musictree provider.
    * @q_errno: Any error that happenend during glyr_get() (same as argument to glyr_get())
    * @normalization: What normalization to apply to artist/album/title; GLYR_NORMALIZE_MODERATE is default.
    *
    * This structure holds all settings used to influence libglyr.
    * You should set all fields glyr_opt_*, refer also to the documentation there to find out their exact meaning.
    *
    * You can safely read from all fields and should free the query with glyr_query_destroy() after use.
    */
    typedef struct _GlyrQuery
    {
        /*< public >*/
        GLYR_GET_TYPE type;

        int number;
        int plugmax;
        int verbosity;
        size_t fuzzyness;

        int img_min_size;
        int img_max_size;

        int parallel;
        int timeout;
        int redirects;

        bool force_utf8;
        bool download;
        float qsratio;

        GLYR_ERROR q_errno;

        GLYR_NORMALIZATION normalization;

        bool db_autoread;
        bool db_autowrite;
        GlyrDatabase * local_db;

        bool lang_aware_only;

        /* Signal conditions */
        volatile int signal_exit;

        /* Dynamic allocated */
        char * lang;
        char * proxy;
        char * artist;
        char * album;
        char * title;
        char * from;
        char * allowed_formats;
        char * useragent;
        char * musictree_path;

#ifndef __GTK_DOC_IGNORE__
        struct
        {
            GLYR_ERROR (* download) (GlyrMemCache * dl, struct _GlyrQuery * s);
            void  * user_pointer;
        } callback;
#endif

        /*< private >*/
        int itemctr; /*!< Do not use! - Counter of already received items - you shouldn't need this */
        char * info[10]; /*!< Do not use! - A register where porinters to all dynamic alloc. fields are saved. Do not use. */
        bool imagejob; /*! Do not use! - Wether this query will get images or urls to them */
        long is_initalized; /* Do not use! - Wether this query was initialized correctly */

    } GlyrQuery;

    /**
     * GlyrSourceInfo:
     * @name: The name of the provider.
     * @key: A one-letter shorter-form of @name
     * @type: Tells what type of data this provider delivers
     * @quality: A quality rating from 0-100
     * @speed: A speed rating form 0
     * @lang_aware: Does this provider offer language specific content?
     * @next: A pointer to the next provider.
     * @prev: A pointer to the previous provider.
     *
     * Represents a provider.
     * It's a simpler version of the internal version,
     * with statically allocated data only,
     * therefore you can modify and read to your liking.
     *
     * It is freed when glyr_info_free() is called on it's GlyrFetcherInfo
     */
    typedef struct _GlyrSourceInfo
    {

        /*< public >*/
        char * name;
        char key;
        GLYR_GET_TYPE type;
        int quality;
        int speed;
        bool lang_aware;

        struct _GlyrSourceInfo * next;
        struct _GlyrSourceInfo * prev;
    } GlyrSourceInfo;

    /**
     * GlyrFetcherInfo:
     * @name: The name of the provider.
     * @type: Tells what type of data this getter delivers.
     * @reqs: A bitmask. You can test if this getter requires a filled artist: (reqs & GLYR_REQUIRES_ARTIST)
     * @head: A doubly linked list of GlyrSourceInfo (the provider available for this getter)
     * @next: A pointer to the next provider.
     * @prev: A pointer to the previous provider.
     *
     * Represents a getter.
     *
     * It's a simpler version of the internal version,
     * therefore you can modify and read to your liking.
     *
     * You should pass it to glyr_info_free() once done.
     *
     * @see_also: glyr_info_get()
     */
    typedef struct _GlyrFetcherInfo
    {

        /*< public >*/
        char * name;
        GLYR_GET_TYPE type;
        GLYR_FIELD_REQUIREMENT reqs;
        GlyrSourceInfo * head;

        struct _GlyrFetcherInfo * next;
        struct _GlyrFetcherInfo * prev;
    } GlyrFetcherInfo;


    /**
     * DL_callback:
     * @dl: The current item you can investigate. Guaranteed to be not #NULL.
     * @s: The GlyrQuery you initially passed to glyr_get()
     *
     * Typedef'd version of the callback option used by glyr_opt_download()
     *
     * Returns: a #GLYR_ERROR
    */
    typedef GLYR_ERROR (*DL_callback) (GlyrMemCache * dl, struct _GlyrQuery * s);

#ifdef __cplusplus
}
#endif

#endif
