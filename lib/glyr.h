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
#ifndef GLYR_H
#define GLYR_H

/**
* @file glyr.h
* @brief All method declaration here. You should only need to include this file.
*
* All stable API of libglyr is right here. 
*
* @author Christopher Pahl
* @version 0.6
* @date 2011-06-14
*/

/* All structs used by glyr are here */
#include "types.h"
	
/* */
#ifdef __cplusplus
extern "C"
{
#endif
/**
* @brief call this at startup. 
* This is not threadsafe and Gly_cleanup should be called once for everytime you call Gly_init
* Actually this method is only there to initialize libcurl, glyr only depends on the settings struct.
*/
        void Gly_init(void);

/**
* @brief Call this at program termination. It's adviseable to call 'atexit(Gly_cleanup)' after Gly_init()
* Not threadsafe also.
*/
        void Gly_cleanup(void);

/**
* @brief The 'main' method of glyr. It starts the searching according to the settings in the settings param 
*
* @param settings The setting struct controlling glyr. (See the GlyOpt_* methods)
* @param error An optional pointer to an int, which gets filled with an error message, or GLYRE_OK on success
* @param length An optional pointer storing the length of the returned list
*
* It takes a pointer to a GlyQuery struct filled to your needs via the GlyOpt_* methods,\n
* Once an item is found the callback (set via GlyOpt_dlcallback) is called with the item as parameter.\n
* After return all items are listed in a GlyCacheList ready to be accessed, remember to delete it with Gly_free_list when done.\n
*
* @return A GlyCacheList containing all found data. See the struct reference for further details.
*/
        GlyMemCache * Gly_get(GlyQuery * settings, enum GLYR_ERROR * error, int * length);

/**
* @brief Init's the GlyQuery structure to sane defaults. 
* Call this after creating the variable.
*
* @param glyrs The fresh GlyQuery to be init'd.
*/
        void Gly_init_query(GlyQuery * glyrs);

/**
* @brief Free all memory associated with this query, and restore default settings.
* Do this always when you're done with this one.
* @param sets The GlyQuery to be destroyed
*/
        void Gly_destroy_query(GlyQuery * sets);

/**
* @brief Free the memory in the GlyCacheList returned by Gly_get
*
* @param head The GlyCacheList to be free'd
*/
	void Gly_free_list(GlyMemCache * head);

/**
* @brief Returns a newly allocated and initialized GlyMemCache, mostly for use with Gly_gtrans_*
* Don't forget to free it with Gly_free_cache
* @return A newly allocated GlyMemCache
*/
        GlyMemCache * Gly_new_cache(void);

/**
* @brief Produces an exact copy of the GlyMemCache source.
*
* @param source The GlyMemCache to copy
*
* @return A newly allocated GlyMemCache
*/
        GlyMemCache * Gly_copy_cache(GlyMemCache * source);

/**
* @brief Free the GlyMemCache pointed to by c. You should set it to NULL also, as using it after this will crash your program. 
*
* @param c An allocated GlyMemCache
*/
        void Gly_free_cache(GlyMemCache * c);

/********************************************************
 * GlyOpt methods ahead - use them to control Gly_get() * 
 ********************************************************/

/**
* @brief Set the callback that is executed once an item is ready downloaded
*
* @param settings The GlyQuery settings struct to store this option in
* @param dl_cb The callback to register, must have a prototype like this:\n
  	       enum GLYR_ERROR my_callback(GlyMemCache * dl, struct GlyQuery * s);
* @param userp A pointer to a custom variable you can access inside the callback via s->callback.user_pointer;
*
* Note that you can return a certain integer in the callback:\n
* GLYRE_IGNORE: To not add this item to the results.
* GLYRE_OK: To add this item to the results and continue happily.
* GLYRE_STOP_BY_CB: To stop right now and return the results. The last element will NOT be added.
*
* @return an errorID
*/
        enum GLYR_ERROR GlyOpt_dlcallback(GlyQuery * settings, DL_callback dl_cb, void * userp);
/**
* @brief What type of metadata to search for. Must be one of GLYR_GET_TYPE enum.
*
* @param s The GlyQuery settings struct to store this option in
* @param type A member of the GLYR_GET_TYPE enum, set this before you set anything else.
*
* @return an errorID
*/
        enum GLYR_ERROR GlyOpt_type(GlyQuery * s, enum GLYR_GET_TYPE type);
/**
* @brief The artist field. libglyr will try to format it to fit the best.
*
* @param s The GlyQuery settings struct to store this option in
* @param artist A nullterminated char, a copy of the string will be held internally so you can savely modify your version.
*
* This field is required for all getters. You are required to fill it.
*
* @return an errorID
*/
        enum GLYR_ERROR GlyOpt_artist(GlyQuery * s, char * artist);
/**
* @brief The album field. libglyr will try to format it to fit the best.
*
* @param s The GlyQuery settings struct to store this option in.
* @param album A nullterminated char, a copy of the string will be held internally so you can savely modify your version.
*
*	Required for the following getters:
*	  - albumlist
*	  - cover
*	  - review 
*	  - tracklist
*
*	Optional for those:
*	  - tags
*	  - relations
*         - lyrics 
*
*	
*
* @return an errorID
*/
        enum GLYR_ERROR GlyOpt_album(GlyQuery * s,  char * album);
/**
* @brief The title field. libglyr will try to format it to fit the best.
*
* @param s The GlyQuery settings struct to store this option in.
* @param title A nullterminated char, a copy of the string will be held internally so you can savely modify your version.
*
*	Required for:
*	  - lyrics
*	Optional for:
*	  - tags
*	  - relations
*
* @return an errorID
*/
        enum GLYR_ERROR GlyOpt_title(GlyQuery * s,  char * title);
/**
* @brief Maximum size a cover may have (assuming the cover is quadratic, only one size is required) 
*
* @param s The GlyQuery settings struct to store this option in.
* @param size The max. size in pixel
*
* Please note: libglyr takes this as a hint, and not as an absolute measure. You may find yourself with slightly oversized or undersized covers,\n 
* but generally overall in the range between cmin and cmax. Also, this only works for the 'cover' getter, not for 'photos'!\n
*
* @return an errorID
*/
        enum GLYR_ERROR GlyOpt_cmaxsize(GlyQuery * s, int size);
/**
* @brief Minimum size a cover may have (assuming the cover is quadratic, only one size is required) 
*
* @param s The GlyQuery settings struct to store this option in.
* @param size The min. size in pixel
*
* Note: Also see GlyOpt_cmaxsize()
* 
* @return an errorID
*/
        enum GLYR_ERROR GlyOpt_cminsize(GlyQuery * s, int size);
/**
* @brief The number of items that may be downloaded in parallel
*
* @param s The GlyQuery settings struct to store this option in.
* @param val the number as unsigned long
*
* @return an errorID
*/
        enum GLYR_ERROR GlyOpt_parallel(GlyQuery * s, unsigned long val);
/**
* @brief Amout of seconds to wait before cancelling an download 
*
* @param s The GlyQuery settings struct to store this option in.
* @param val Timeout in seconds.
*
* If more than one item is downloaded in parallel, the timeout will be changed accordingly.\n
* Default is 20 seconds.
*
* @return an errorID 
*/
        enum GLYR_ERROR GlyOpt_timeout(GlyQuery * s, unsigned long val);
/**
* @brief Max number of redirects to 
*
* @param s The GlyQuery settings struct to store this option in.
* @param val an unsigned integer  
* 
* A value of 0 is allowed, but may break certain plugins.\n
* Default = 1
*
* @return an errorID 
*/
        enum GLYR_ERROR GlyOpt_redirects(GlyQuery * s, unsigned long val);
/**
* @brief Set the language the items should be in.
*
* @param s The GlyQuery settings struct to store this option in.
* @param langcode
*	The language used for providers with multilingual content.
*	It is given in ISO-639-1 codes, i.e 'de','en','fr' etc.	
*	
*	List of providers recognizing this option:\n
*	   * cover/amazon (which amazon server to query)
*	   * cover/google (which google server to query)
*	   * ainfo/lastfm (the language the biography shall be in)\n
*
*	(Use only these providers if you really want ONLY localized content)\n
*	If no language specified the language defaults to english ("en")
*
* Note1: This only works with a few providers, which should be set via GlyOpt_from()
* Note2: Don't coinfuse this with the built-in google translator's settings.
*
* @return an errorID 
*/
        enum GLYR_ERROR GlyOpt_lang(GlyQuery * s, char * langcode);
/**
* @brief Set the number of items to search.
*
* @param s The GlyQuery settings struct to store this option in.
* @param num the number as an integer
*
*	How many items to search for (1 to INT_MAX)\n
*	This is not the number of items actually returned then,\n
*	because libglyr is not able to find 300 songtexts of the same song,\n
*	or libglyr filters duplicate items before returning.\n
*
* @return an errorID
*/
        enum GLYR_ERROR GlyOpt_number(GlyQuery * s, unsigned int num);
/**
* @brief Set libglyr's verbosity level (debug) 
*
* @param s The GlyQuery settings struct to store this option in.
* @param level The level as an integer, see description below
*
*        0) nothing but fatal errors.\n
*        1) warnings and important notes.\n
*        2) normal, additional information what libglyr does.\n
*        3) basic debug output.\n
*        4) libcurl debug output.\n
*
* @return an errorID
*/
        enum GLYR_ERROR GlyOpt_verbosity(GlyQuery * s, unsigned int level);
/**
* @brief Define the providers you want to use 
*
* @param s The GlyQuery settings struct to store this option in.
* @param from a string, see below
*
*        Use this to define what providers you want to use.\n
*        Every provider has a name and a key which is merely a shortcut for the name.\n
*        Specify all providers in a semicolon seperated list.\n
*        Type 'glyrc -H' for a complete list of all providers for each getter.\n
*
*          Example:\n
*            "amazon;google" \n
*           "a;g" - same with keys\n
*
*        You can also prepend each word with a '+' or a '-' ('+' is assumend without),\n
*        which will add or remove this provider from the list respectively.\n
*        Additionally you may use the predefined groups 'safe','unsafe','fast','slow','special'.\n
*        
*          Example:\n
*           "+fast;-amazon" which will enable last.fm and lyricswiki.\n
*
*
* @return an errorID
*/
        enum GLYR_ERROR GlyOpt_from(GlyQuery * s, const char * from);
/**
* @brief Enable colored log output. (Debug) 
*
* @param s The GlyQuery settings struct to store this option in.
* @param iLikeColorInMyLife a boolean
*
*	Colored output only works in terminal with standard color support,\n
*	which means, non standard terminals like the MS commandline do not work.
*
* @return always GLYRE_OK
*/
        enum GLYR_ERROR GlyOpt_color(GlyQuery * s, bool iLikeColorInMyLife);
/**
* @brief Define the maximum number of items a provider may download 
*
* @param s The GlyQuery settings struct to store this option in.
* @param plugmax
*
*	Use this to scatter the results over more providers, to get different results.\n
*	You can set it also to -1 what allows an infinite number of items (=> default)
*
* @return an errorID 
*/
        enum GLYR_ERROR GlyOpt_plugmax(GlyQuery * s, int plugmax);
/**
* @brief Define if image items (i.e, covers, photos) are downloaded.
*
* @param s The GlyQuery settings struct to store this option in.
* @param download
*
*        For image getters only.\n
*        If set to true images are also coviniently downloaded and returned.\n
*        Otherwise, just the URL is returned for your own use.\n
*
*        Default to 'true', 'false' would be a bit more searchengine like.\n
*
* @return an errorID 
*/
        enum GLYR_ERROR GlyOpt_download(GlyQuery * s, bool download);
/**
* @brief Define wether searching happens in groups or everything or parallel (= false)
*
* @param s The GlyQuery settings struct to store this option in.
* @param groupedDL boolean to toggle this
*
*        If set false, this will disable the grouping of providers.\n
*        By default providers are grouped in categories like 'safe','unsafe','fast' etc., which\n
*        are queried in parallel, so the 'best' providers are queried first.\n
*        Disabling this behaviour will result in increasing speed, but as a result the searchresults\n
*        won't be sorted by quality, as it is normally the case.\n
*
* @return an errorID
*/
        enum GLYR_ERROR GlyOpt_groupedDL(GlyQuery * s, bool groupedDL);
/**
* @brief Define allowed image formats 
*
* @param s The GlyQuery settings struct to store this option in.
* @param formats A comma seperated list of format specifiers, e.g. "png;jpg;jpeg"
*
* 
*       Awaits a string with a semicolon seperated list of allowed formats.\n
*       The case of the format is ignored.\n
*
*        Example:\n
*           "png;jpg;jpeg" would allow png and jpeg.\n
*
*        You can also specify "all", which disables this check.\n
*
* @return an errorID
*/
        enum GLYR_ERROR GlyOpt_formats(GlyQuery * s, const char * formats);
/**
* @brief Set the max. tolerance for fuzzy matching 
*
* @param s The GlyQuery settings struct to store this option in.
* @param fuzz
*
*	Set the maximum amount of inserts, edits and substitutions, a search results\n
*       may differ from the artist and/or album and/or title.\n
*       The difference between two strings is measured as the 'Levenshtein distance',\n
*       i.e, the total amount of inserts,edits and substitutes needed to convert string a to b.\n
*
*       Example:\n
*          "Equilibrium" <=> "Aqilibriums" => Distance=3\n
*          With a fuzzyness of 3 this would pass the check, with 2 it won't.\n
*        
*       Higher values mean more search results, but more inaccuracy. \n
*       Default is 4.
*
* @return an errorID 
*/
        enum GLYR_ERROR GlyOpt_fuzzyness(GlyQuery * s, int fuzz);
/**
* @brief Enable/Disable check for duplicate items. 
*
* @param s The GlyQuery settings struct to store this option in.
* @param duplcheck boolean flag, true enables, false disables
*
*	Actually there is no valid reason to set this to false,\n
*	except duplicate items are okay for you.
*
* @return an errorID 
*/
        enum GLYR_ERROR GlyOpt_duplcheck(GlyQuery * s, bool duplcheck);

/**
* @brief Set the source language for google translator or NULL to autodetect  
*
* @param s The GlyQuery settings struct to store this option in.
* @param source A nullterminated pointer to char.
*
*	You can use 'glyrc gtrans list' or the Gly_gtrans_list() method,\n
*	to get a list of all valid values.
*
* @return an errorID
*/
	enum GLYR_ERROR GlyOpt_gtrans_source_lang(GlyQuery * s, const char * source);
/**
* @brief Set the target language for google translator or NULL to disable
*
* @param s The GlyQuery settings struct to store this option in.
* @param target
*
*	By default this feature is disabled (target == NULL)\n
*	Use the methods in lib/translate.h to use the corresponding methods directly.
*	
*
* @return an errorID 
*/
	enum GLYR_ERROR GlyOpt_gtrans_target_lang(GlyQuery * s, const char * target);

            
/**
* @brief Set the proxy to use
*
* @param s The GlyQuery settings struct to store this option in.
* @param proxystring the proxy setting.
*
*  NULL for none, otherwise see the documentation of curl_easy_setopt(CURLOPT_PROXY) how to set this.
*  Synatx: [protocol://][user:pass@]Domain[:port] 
*  Example: "http://Proxy.fh-hof.de:3128"
* 
* If empty the global env $http_proxy shall be used, if present.
*
* @return an errorID
*/
	enum GLYR_ERROR GlyOpt_proxy(GlyQuery * s, const char * proxystring);

/**
* @brief Get a nullterminated list of available providers 
*
* @param ID What type to get, or GET_UNSURE for a list of getters
*
* A list is returned with all providernames for this type.\n
* Example:\n
* {"last.fm","amazon",...,"coverhunt",NULL}; \n
* You can use this in GlyOpt_from()\n
*
* @return a nullterminated list of available providers
*/
	const char ** GlyPlug_get_name_by_id(enum GLYR_GET_TYPE ID);

/**
* @brief Same as GlyPlug_get_name_by_id, but returns single string
*
* @param ID What type to get, or GET_UNSURE for a list of getters
*
* @return a nullterminated string of providers, seperated with '|'
*/
  const char * GlyPlug_get_single_name_by_id(enum GLYR_GET_TYPE ID);

/**
* @brief Get a nullterminated list of available providers, with their name as key
*
* @param ID What type to get, or GET_UNSURE for a list of getters
*
* A string is returned with all keys of this getter.\n
* Example: \n
* "lawgbdmc\\0", where 'a' stands for amazon.\n
* You can use them (or their full names is GlyOpt_from()\n
*
* @return a nullterminated list of available providers
*/
	const char *  GlyPlug_get_key_by_id(enum GLYR_GET_TYPE ID);
/**
* @brief Get a list of GroupIDs 
*
* @param ID What type to get, or GET_UNSURE for a list of getters
*
* You can use the groupid to determine in whatt group this provider is.\n
* You will not use this function in 99.9% of all cases, but it can be useful to translate user input\n
* to glyr's represantation, or to list groups as glyrc does. 
* GET_UNSURE will result in a list of all GET_* enumerators (i.e. GET_COVER...GET_UNSURE)\n
*
* @return ..
*/
	char * GlyPlug_get_gid_by_id(enum GLYR_GET_TYPE ID);

/**
* @brief A convinience method to download the content at the URl $url, according to the settings in $s 
* Actually only the redirect and timeout parameters are used here. 
*
* @param url The url to download as nullterminated string. Must be a vaild URL.
* @param s A GlyQuery with the timeout and redirect values filled to your needs.
*
* @return A GlyMemCache containing the data
*/
        GlyMemCache * Gly_download(const char * url, GlyQuery * s);

/**
* @brief Many methods use an returnvalue, or error parameters to inform you about errors
* Use this method to get a descriptive message you can print.
*
* @param ID The returned error
*
* @return A descriptive nullterminated string, do not pass to free 
*/
        const char * Gly_strerror(enum GLYR_ERROR ID);

/**
* @brief Get the name of a group as string
*
* @param ID a member of the GLYR_GROUPS enum
*
* GRP_FAST gets to "fast",GRP_ALL to "all" etc.\n
* Use this to make strings from GlyPlug_get_gid_by_id()\n
*
* @return A groupname as string or NULL if not found
*/
        const char * Gly_groupname_by_id(enum GLYR_GROUPS ID);

/**
* @brief Returns versioning information, including compiletime
*
*  Example: \n
*    Version 0.4 (Larcenous Locust (dev)) of [May 20 2011] compiled at [19:12:37]
*
* @return A nullterminated string, do not free 
*/
        const char * Gly_version(void);

/**
* @brief Writes data to a specified path
*
* @param data The data to write. 
* @param path The path to write data at.
*
* Writes data to path $path, special values for $path can be 'stdout','stderr' or 'null',\n
* which are pretty selfexplaining.
* 
* @return An error id.
*/
        int Gly_write(GlyMemCache * data, const char * path);


/**
* @brief Updates the md5sum field of the cache c 
*
* @param c a valid memcahe
*/
	void Gly_update_md5sum(GlyMemCache * c);

#ifdef _cplusplus
}
#endif

#endif
