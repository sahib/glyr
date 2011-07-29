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
#define DEFAULT_TIMEOUT  20L /*! default timeout */
#define DEFAULT_REDIRECTS 1L /*! default redirects */
#define DEFAULT_PARALLEL 4L  /*! default parallel */
#define DEFAULT_CMINSIZE 125 /*! default minsize of cover */
#define DEFAULT_CMAXSIZE -1
#define DEFAULT_VERBOSITY 0
#define DEFAULT_NUMBER 1
#define DEFAULT_PLUGMAX -1
#define DEFAULT_LANG "en"
#define DEFAULT_DOWNLOAD true
#define DEFAULT_GROUPEDL true
#define DEFAULT_FROM_ARGUMENT_DELIM ";"
#define DEFAULT_DUPLCHECK true
#define DEFAULT_FUZZYNESS 4
#define DEFAULT_PROXY NULL 

/* --------------------------- */
/* --------- GROUPS ---------- */
/* --------------------------- */

/**
* @file types.h
* @brief File containing all definitions used in the external API
*
* All defines/typedefs/enums/structs are here.
* Onyl dependency is stdlib/stdbool 
*
* @author Christopher Pahl
* @version 0.6
* @date 2011-06-14
*/

/**
* @brief All possible errors that may be returned
*/
enum GLYR_ERROR {
        GLYRE_OK,           /*!< everything is fine */
        GLYRE_BAD_OPTION,   /*!< you passed a bad option to Gly_setopt() */
        GLYRE_BAD_VALUE,    /*!< Invalid value in va_list */ 
        GLYRE_EMPTY_STRUCT, /*!< you passed an empty struct to Gly_setopt() */
        GLYRE_NO_PROVIDER,  /*!< setttings->provider == NULL */ 
        GLYRE_UNKNOWN_GET,  /*!< settings->type is not valid */ 
        GLYRE_IGNORE,       /*!< If returned by callback, cache is ignored */
        GLYRE_STOP_BY_CB    /*!< Callback returned stop signal. */
};

/**
* @brief Enumeration of all getters, GlyQuery is initalized to GET_UNSURE
*
*  The type of metadata to get, names are selfexplanatory
*  Requirements are given in braces, [] means optional.
*/
enum GLYR_GET_TYPE {
        GET_COVERART = 1, /*!< Get coverart for (artist|album) */
        GET_LYRICS = 2, /*!< Get lyrics for (artist|[album]|artist) */
        GET_ARTIST_PHOTOS = 3, /*!< Get pics for (artist) */
        GET_ARTISTBIO = 4, /*!< Get bio of (artist) */
        GET_SIMILIAR_ARTISTS = 5, /*!< Get similiar artists to (artist) */
        GET_SIMILIAR_SONGS = 6, /*!< Get similiar songs (artist|title) */
        GET_ALBUM_REVIEW = 7, /*!< Get album review to (artist|album) */
        GET_TRACKLIST = 8, /*!< Get list of tracks for album (artist|album) */
        GET_TAGS = 9, /*!< Get tags (i.e. words like 'Metal') for (artist|[album]|[title]) */
        GET_RELATIONS = 10, /*!< Get relations (e.g. links to last.fm/wikipedia) for (artist|[album]|[title]) */
        GET_ALBUMLIST= 11, /*!< Get a list of albums by (artist) */
        GET_UNSURE = 12 /*!< Do nothing but relax */
};

/**
* @brief All possible values the type field of GlyMemCache can have
*/
enum GLYR_DATA_TYPE {
        TYPE_NOIDEA, /*!< You shouldn't get this */
        TYPE_LYRICS, /*!< Lyrics. */
        TYPE_REVIEW, /*!< Album reviews */
        TYPE_PHOTOS, /*!< Pics showing a certain band */
        TYPE_COVER,  /*!< Coverart */
        TYPE_COVER_PRI, /*!< A cover known to be the front side of the album */
        TYPE_COVER_SEC, /*!< A cover known to be the backside, inlet etc. */
        TYPE_AINFO,  /*!< Artist bio */
        TYPE_SIMILIAR, /*!< Similiar artists */
        TYPE_SIMILIAR_SONG, /*!< Similar songs */
        TYPE_ALBUMLIST, /*!< List of albums, each cache containing one name */
        TYPE_TAGS,	/*!< List of (random) tags, each cache containing one name */
        TYPE_TAG_ARTIST,/*!< Tag associated with the artist */
        TYPE_TAG_ALBUM, /*!< Tag associated with the album */
        TYPE_TAG_TITLE, /*!< Tag associated with the album */
        TYPE_RELATION,  /*!< Random relation, each cache containing one link */
	TYPE_IMG_URL,   /*!< URL pointing to an image */
	TYPE_TXT_URL,   /*!< URL pointing to some text content */
        TYPE_TRACK	/*!< List of tracknames, each cache containing one name */
};


/**
* @brief Represents a single item.
*
* It's used all over the program, and is the actual struct you're working with and you're wanting from libglyr. 
*/
typedef struct GlyMemCache {
        char  *data;    /*!< Data buffer, you can safely read this field, but remember to update the size field if you change it and to free the memory if needed. */
        size_t size;    /*!< Size of data, cahnge this if you changed the data field. */
        char  *dsrc;    /*!< Source of data, i.e. an exact URL to the place where it has been found. */
        char  *prov;    /*!< The name of the provider which found this item */
        int   type;     /*!< The metadata type, is one of the GLYR_GET_TYPE enum */
        int   error;    /*!< error code - internal use only */
        int   duration; /*!< Duration of a song (in seconds). Only filled for the tracklist getter. */
        bool  is_image; /*!< Wether it is an image or a textitem */
        unsigned char md5sum[16]; /*!< A checksum of generated from the data field, used internally for duplicate checking, useable as identifier from data */

	/* Linkage */
	struct GlyMemCache * next; /*!< Pointer to next cache in list, or NULL */
	struct GlyMemCache * prev; /*!< Pointer to prev cache in list, or NULL */
} GlyMemCache;


/**
* @brief Structure controlling all of libglyr's options
* 
* You should modify this with the GlyOpt_* methods,\n
* You can read all members directly.\n
* Look up the corresponding GlyOpt_$name methods for more details.
* For reading: Dynamically allocated members are stored in '.alloc'! 
*/
typedef struct GlyQuery {
        enum GLYR_GET_TYPE type; /*!< What type of data to get */

        int number; /*!< Number of items to download */
        int plugmax; /*!< Number of items a single provider may download */
        int verbosity; /*!<See GlyOpt_verbosity() for all levels */
        size_t fuzzyness; /*!< Treshold for Levenshtein algorithm */

	/**
	* @brief Anonymous Struct holding min/maxsize for covers
	*/
        struct {
                int min_size; /*!< Min size a cover may have */
                int max_size; /*!< Max size a cover may have */
        } cover;


        long parallel; /*!< Max parallel downloads */
        long timeout;  /*!< Max timeout for downloads */
        long redirects;/*!< Max redirects for downloads */

        bool color_output;
        bool download; /*!< return only urls without downloading, converting glyr to a sort of search engine */
        bool groupedDL; /*!< Download group for group, or all in parallel (faster, but less accurate) */
        bool duplcheck; /*!< Check for bad data? */

#ifdef COMING_FROM_SWIG
  /* Make this fields immutable for languaging supporting it */
  %immutable {
#endif

	/* Callback and userpointer */
        struct {
           enum GLYR_ERROR (* download)(GlyMemCache * dl, struct GlyQuery * s);
           void  * user_pointer;
        } callback;
  
/**
* @brief anonymous struct holding the source and target lang for gtrans
* If source is NULL, autodetection will be enabled,\n
* target has to be !NULL to enable translating
*/

#ifdef COMING_FROM_SWIG
  %}
#endif
	/* Dynamic allocated */
        const char * lang; /*!< language settings (for amazon / google / last.fm) */
	const char * proxy; /*!< Proxy settings */
        char * artist; /*!< artist field */
        char * album;  /*!< album field */
        char * title;  /*!< title field */
        char * from;   /*!< String passed to GlyOpt_from() */

	
	char * info[10]; /*!<A register where porinters to all dynamic alloc. fields are saved. Do not use. */

} GlyQuery;

/**
* @brief typefef for the GlyOpt_dlcallback() option
*
* @param DL_callback A callback of the form: enum GLYR_ERROR cb(GlyMemCache * dl, struct GlyQuery * s)
*
* @return possibly an error or GLYRE_OK 
*/
typedef enum GLYR_ERROR (*DL_callback)(GlyMemCache * dl, struct GlyQuery * s);

#ifdef COMING_FROM_SWIG
	%extend GlyQuery
	{
		GlyQuery()
		{
			GlyQuery my_query; 
			Gly_init_query(&my_query);
			GlyQuery * copy = malloc(sizeof(GlyQuery));
			memcpy(copy,&my_query,sizeof(GlyQuery));
			return copy;
		}
		~GlyQuery()
		{
			Gly_destroy_query($self);
			if($self != NULL)
			    free($self);
		}
	}

	%extend GlyMemCache
	{
		GlyMemCache()
		{
			return Gly_new_cache();
		}
		~GlyMemCache()
		{
			Gly_free_cache($self);
		}
	}
#endif


#endif
