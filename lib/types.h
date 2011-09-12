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

#include <stdlib.h>
#include <stdbool.h>

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
#define GLYR_DEFAULT_SUPPORTED_LANGS "en|de|fr|es|it|jp|pl|pt|ru|sv|tr|zh"

/* Disallow *.gif, mostly bad quality
 * jpeg and jpg, because some not standardaware
 * servers give MIME types like image/jpg
 * (which should be image/jpeg usually)
 */
#define GLYR_DEFAULT_ALLOWED_FORMATS "png;jpeg;tiff;jpg;"

/* Be honest by default */
#define GLYR_DEFAULT_USERAGENT "libglyr/0.85"

/* --------------------------- */
/* --------- GROUPS ---------- */
/* --------------------------- */

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
    GLYRE_NO_INIT     
}   GLYR_ERROR;

/**
* GLYR_GET_TYPE:
* @GLYR_GET_COVERART: Get Coverart.
* @GLYR_GET_LYRICS:  Get Songtext
* @GLYR_GET_ARTIST_PHOTOS: Get Live/Promotion/Fanartwork
* @GLYR_GET_ARTISTBIO: Get information about this artist
* @GLYR_GET_SIMILIAR_ARTISTS: get similar artists
* @GLYR_GET_SIMILIAR_SONGS: get similar songs
* @GLYR_GET_ALBUM_REVIEW: get album reviews
* @GLYR_GET_TRACKLIST: get a tracklist for a specified album
* @GLYR_GET_TAGS: get tags for a certain item
* @GLYR_GET_RELATIONS: get relations for a certain item
* @GLYR_GET_ALBUMLIST: get a list of albums from a certain artist
* @GLYR_GET_GUITARTABS: get guitar tabs for a specified artist/title
* @GLYR_GET_UNSURE: The default value after initializing a query.
* 
* You tell libglyr what metadata you want by choosing one of the below
* and set it via glyr_opt_type()
*
*/
typedef enum
{
    GLYR_GET_UNSURE, 
    GLYR_GET_COVERART, 
    GLYR_GET_LYRICS, 
    GLYR_GET_ARTIST_PHOTOS, 
    GLYR_GET_ARTISTBIO, 
    GLYR_GET_SIMILIAR_ARTISTS, 
    GLYR_GET_SIMILIAR_SONGS,
    GLYR_GET_ALBUM_REVIEW, 
    GLYR_GET_TRACKLIST, 
    GLYR_GET_TAGS, 
    GLYR_GET_RELATIONS, 
    GLYR_GET_ALBUMLIST, 
    GLYR_GET_GUITARTABS
}   GLYR_GET_TYPE;

/**
* GLYR_DATA_TYPE:
* @GLYR_TYPE_NOIDEA: You shouldn't get this 
* @GLYR_TYPE_LYRICS: Songtext
* @GLYR_TYPE_REVIEW: Albumreview
* @GLYR_TYPE_PHOTOS: Pictures showing a certain band
* @GLYR_TYPE_COVER: coverart
* @GLYR_TYPE_COVER_PRI:  A cover known to be the front side of the album 
* @GLYR_TYPE_COVER_SEC:  A cover known to be the backside: inlet etc. 
* @GLYR_TYPE_AINFO: Artist bio 
* @GLYR_TYPE_SIMILIAR: Similiar artists 
* @GLYR_TYPE_SIMILIAR_SONG: Similar songs 
* @GLYR_TYPE_ALBUMLIST: List of albums: each cache containing one name 
* @GLYR_TYPE_TAGS: List of (random) tags: each cache containing one name 
* @GLYR_TYPE_TAG_ARTIST: Tag associated with the artist 
* @GLYR_TYPE_TAG_ALBUM: Tag associated with the album 
* @GLYR_TYPE_TAG_TITLE: Tag associated with the album 
* @GLYR_TYPE_RELATION: Random relation: each cache containing one link 
* @GLYR_TYPE_IMG_URL: URL pointing to an image 
* @GLYR_TYPE_TXT_URL: URL pointing to some text content 
* @GLYR_TYPE_TRACK: List of tracknames: each cache containing one name 
* @GLYR_TYPE_GUITARTABS: Guitartabs
* 
* Mainly used in the 'type' field of GlyrMemCache.
* It describes what kind of data the cache holds.
* 
*/
typedef enum
{
    GLYR_TYPE_NOIDEA, 
    GLYR_TYPE_LYRICS, 
    GLYR_TYPE_REVIEW, 
    GLYR_TYPE_PHOTOS, 
    GLYR_TYPE_COVER,  
    GLYR_TYPE_COVER_PRI, 
    GLYR_TYPE_COVER_SEC, 
    GLYR_TYPE_AINFO,  
    GLYR_TYPE_SIMILIAR, 
    GLYR_TYPE_SIMILIAR_SONG, 
    GLYR_TYPE_ALBUMLIST, 
    GLYR_TYPE_TAGS,	
    GLYR_TYPE_TAG_ARTIST,
    GLYR_TYPE_TAG_ALBUM, 
    GLYR_TYPE_TAG_TITLE, 
    GLYR_TYPE_RELATION,  
    GLYR_TYPE_IMG_URL,   
    GLYR_TYPE_TXT_URL,   
    GLYR_TYPE_TRACK,	
    GLYR_TYPE_GUITARTABS,
}   GLYR_DATA_TYPE;

/**
 * GlyrMemCache:
 * @data: contains the data, string when is_image is false, raw data otherwise
 * @size: Size of this item in bytes
 * @dsrc: URL pointing to the origin of this item.
 * @prov: name of the provider that delivered this item.
 * @type: The #GLYR_DATA_TYPE of this item.
 * @duration: For tracklist, only. Contains the tracklength in seconds.
 * @is_image: Is this item an image?
 * @img_format: Format of the image (png,jpeg), NULL if text item.
 * @md5sum: A md5sum of the data field.
 * @next: A pointer to the next item in the list, or NULL
 * @prev: A pointer to the previous item in the list, or NULL
 *
 * GlyrMemCache represents a single item received by libglyr. 
 * You should <emphasis>NOT</emphasis> any of the fields, it is meant to be read-only.
 */
typedef struct _GlyrMemCache {

  /*< public >*/
  char  *data;        
  size_t size;        
  char  *dsrc;        
  char  *prov;        
  GLYR_DATA_TYPE type;
  int   duration;     
  bool  is_image;    
  char * img_format; 
  unsigned char md5sum[16]; 

  struct _GlyrMemCache * next; 
  struct _GlyrMemCache * prev; 
} GlyrMemCache;

/**
* GlyrQuery:
* @type: The type of metadata to get.
* @number: The maximum number of items to get; 0 -> inf
* @plugmax: Max number of items per provider.
* @verbosity: How verbose this query should be treated.
* @fuzzyness: Max. treshold for levenshtein.
* @img_min_size: Min. size in pixels an image may have.
* @img_max_size: Min. size in pixels an image may have.
* @parallel: Max. number of parallel queried providers.
* @timeout: Max. timeout in seconds to wait before cancelling a download.
* @redirects: Max number of redirects. You shouldn't set this.
* @force_utf8: Should be UTF8 forced on text items?
* @download: should be images downloaded?
* @qsratio: 0.0 = maxspeed, 1.0 = max quality, 0.85 -> default.
* @lang: Language code ISO-639-1, like 'de','en' or 'auto'
* @proxy: The proxy to use.
* @artist: Artist to use.
* @album: Album to use.
* @title: Title to use.
* @from: Define what providers are queried.
* @allowed_formats: Allowed imageformats.
* @useragent: Useragent to use during http-requests
*
* This structure holds all settings used to influence libglyr.
* You should set all fields glyr_opt_*, refer also to the documentation there to find out their exact meaning.
*
* You can safely read from all fields and should free the query with glyr_destroy_query() after use.
*/
typedef struct _GlyrQuery {
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


/* This is confusing gtk-doc */
#ifndef __GTK_DOC_IGNORE__
	#ifdef COMING_FROM_SWIG
	    /* Make this fields immutable for languages supporting it */
	    %immutable
	    {
	#endif
#endif

    /* Dynamic allocated */
    const char * lang; 
    const char * proxy; 
    char * artist; 
    char * album;  
    char * title;  
    char * from;   
    char * allowed_formats; 
    char * useragent; 

#ifndef __GTK_DOC_IGNORE__
    struct {
         GLYR_ERROR (* download)(GlyrMemCache * dl, struct _GlyrQuery * s);
         void  * user_pointer;
    } callback;
#endif 

    /*< private >*/
    int itemctr; /*!< Do not use! - Counter of already received items - you shouldn't need this */
    char * info[10]; /*!< Do not use! - A register where porinters to all dynamic alloc. fields are saved. Do not use. */
    bool imagejob; /*! Do not use! - Wether this query will get images or urls to them */

/* This is confusing gtk-doc */
#ifndef __GTK_DOC_IGNORE__
	#ifdef COMING_FROM_SWIG
		%}
	#endif
#endif 

} GlyrQuery;

/**
 * GlyrSourceInfo:
 * @name: The name of the provider.
 * @key: A one-letter shorter-form of @name
 * @type: Tells what type of data this provider delivers
 * @quality: A quality rating from 0-100
 * @speed: A speed rating form 0
 * @next: A pointer to the next provider.
 * @prev: A pointer to the previous provider.
 *
 * Represents a provider. 
 * It's a simpler version of the internal version,
 * with statically allocated data only,
 * therefore you can modify and read to your liking.
 *
 * It is freed when glyr_free_plugin_info() is called on it's GlyrFetcherInfo
 */
typedef struct _GlyrSourceInfo {

  /*< public >*/
  char * name;
  char key;
  GLYR_GET_TYPE type;
  int quality;
  int speed;

  struct _GlyrSourceInfo * next;
  struct _GlyrSourceInfo * prev;
} GlyrSourceInfo;

/**
 * GlyrFetcherInfo:
 * @name: The name of the provider.
 * @type: Tells what type of data this getter delivers.
 * @head: A doubly linked list of GlyrSourceInfo (the provider available for this getter)
 * @next: A pointer to the next provider.
 * @prev: A pointer to the previous provider.
 *
 * Represents a getter.
 * 
 * It's a simpler version of the internal version,
 * therefore you can modify and read to your liking.
 * 
 * You should pass it to glyr_free_plugin_info() once done.
 * 
 * @see_also: glyr_get_plugin_info()
 */
typedef struct _GlyrFetcherInfo {

  /*< public >*/
  char * name;
  GLYR_GET_TYPE type;
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
typedef GLYR_ERROR (*DL_callback)(GlyrMemCache * dl, struct _GlyrQuery * s);


/* This is confusing gtk-doc */
#ifndef __GTK_DOC_IGNORE__

/*
 * SWIG STUFF
 * Usually not worth viewing.
 */

	#ifdef COMING_FROM_SWIG
	%extend GlyrQuery
	{
	    GlyrQuery()
	    {
		GlyrQuery my_query;
		glyr_init_query(&my_query);
		GlyrQuery * copy = malloc(sizeof(GlyrQuery));
		memcpy(copy,&my_query,sizeof(GlyrQuery));
		return copy;
	    }
	    ~GlyrQuery()
	    {
		glyr_destroy_query($self);
		if($self != NULL)
		    free($self);
	    }
	}

	%extend GlyrMemCache
	{
	    GlyrMemCache()
	    {
		return glyr_new_cache();
	    }
	    ~GlyrMemCache()
	    {
		glyr_free_cache($self);
	    }
	}

	%extend GlyrFetcherInfo
	{
	    GlyrFetcherInfo()
	    {
		return glyr_get_plugin_info();
	    }

	    ~GlyrFetcherInfo()
	    {
		glyr_free_plugin_info($self);
	    }
	}
	#endif
#endif


#endif
