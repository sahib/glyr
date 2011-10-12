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

/* All structs used by glyr are here */
#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * glyr_init:
 *
 * Init the library, this has to be called before any other calls from this library are made.
 *
 * You should call glyr_cleanup() once for every call of glyr_init()
 * <note>
 * <para>
 * This function is not threadsafe.
 * </para>
 * </note>
 **/
void glyr_init(void);

/**
 * glyr_cleanup:
 *
 * Cleanup all parts of the library, you can use <function>atexit()</function>:
 *
 *
 * <informalexample>
 * <programlisting>
 * glyr_init();
 * atexit(glyr_destroy);
 * </programlisting>
 * </informalexample>
 *
 * <note>
 * <para>
 * This function is not threadsafe.
 * </para>
 * </note>
 **/
void glyr_cleanup(void);


/**
 * glyr_get:
 * @settings: The setting struct controlling glyr. (See the glyr_opt_* methods)
 * @error:  An optional pointer to an int, which gets filled with an error message, or GLYRE_OK on success, or %NULL
 * @length: length An optional pointer storing the length of the returned list, or %NULL
 *
 * @settings is pointer to a #GlyrQuery struct filled to your needs via the glyr_opt_* methods,
 *
 * Once an item is found the callback (set via glyr_opt_dlcallback()) is called anytime a item is ready
 * 
 *
 * Returns:: a doubly linked list of #GlyrMemCache, which should be freed by passing any element of the to glyr_free_list()
 *
 */
GlyrMemCache * glyr_get(GlyrQuery * settings, GLYR_ERROR * error, int * length);

/**
 * glyr_query_init:
 * @query: The GlyrQuery to initialize to defaultsettings.
 *
 * This functions may allocate dynamic memory. It should be freed with glyr_query_init() after use.
 * 
 */
void glyr_query_init(GlyrQuery * query);

/**
 * glyr_query_destroy:
 * @query: The GlyrQuery to destroy.
 *
 * Deletes all modifications and frees dynamic memory. It can be reused, as fresh from glyr_query_init()
 * 
 */
void glyr_query_destroy(GlyrQuery * query);

/**
 * glyr_free_list:
 * @head: The head of the doubly linked list that should be freed.
 *
 * Deletes all dynamic memory by calling glyr_cache_free() on each cache.
 * 
 */
void glyr_free_list(GlyrMemCache * head);

/**
 * glyr_cache_new:
 *
 * Initializes a new memcache.
 *
 * Normally you never need to do this.
 *
 * Don't forget to free the cache with glyr_cache_free()
 *
 * Returns:: A newly allocated and initialized memcache with no data. 
 */
GlyrMemCache * glyr_cache_new(void);

/**
 * glyr_cache_free:
 * @cache: Frees the (valid allocated) cache pointed to by @cache
 */
void glyr_cache_free(GlyrMemCache * cache);

/** 
* glyr_cache_copy:
* @cache: The cache to copy
* 
* Allocate a new cache and 
* copy all contents (= deep copy) from the original @cache,
* The pointers next and prev are set to NULL.
* 
* Returns: A newly allocated cache.
*/
GlyrMemCache * glyr_cache_copy(GlyrMemCache * cache);

/**
* glyr_cache_set_data:
* @cache: The cache where to set the data.
* @data: The data
* @len: Length of data
*
* Safely sets the data of the cache. It frees the old data first, updates 
* the checksum and adjusts the size fields accordingly to len.
* If len is a negative number strlen() is used to determine the size.
* Note: @data is set directly! It get's freed once you free the cache. Be sure it's safe to be free'd.
*
*/
void glyr_cache_set_data(GlyrMemCache * cache, const char * data, int len);

/**
* glyr_cache_write:
* @cache: The data to write.
* @path: The path to write data at.
*
* Write @cache to the path specified by @path.
*
* There are three special files:
* <itemizedlist>
* <listitem>
* <para>
* "stdout" -> Outputs file to stdout
* </para>
* </listitem>
* <listitem>
* <para>
* "stderr" -> Outputs file to stderr
* </para>
* </listitem>
* <listitem>
* <para>
* "null"   -> Outputs item nowhere
* </para>
* </listitem>
* </itemizedlist>
*
* Returns: the number of written bytes.
*/
int glyr_cache_write(GlyrMemCache * cache, const char * path);


/**
* glyr_cache_update_md5sum:
* @cache: a valid memcahe
*
* Updates the md5sum field of @cache. 
*
*/
void glyr_cache_update_md5sum(GlyrMemCache * cache);


/**
* glyr_cache_print:
* @cache: The GlyrMemCache to be printed.
*
* A debug method to print all fields of @cache. 
*
*/
void glyr_cache_print(GlyrMemCache * cache);

/********************************************************
* GlyOpt methods ahead - use them to control glyr_get() *
********************************************************/

/**
* glyr_opt_dlcallback:
* @settings: The GlyrQuery settings struct to store this option in.
* @dl_cb: The callback to register, must have a prototype like this.
* @userp: A pointer to a custom variable you can access inside the callback via <structfield>s->callback.user_pointer</structfield>
*
* The callback should have the following form:
* <informalexample>
* <programlisting>
* GLYR_ERROR my_callback(GlyrMemCache * dl, struct GlyrQuery * s);
* </programlisting>
* </informalexample>
* 
* Note that you can return certaing members of %GLYR_ERROR in the callback:

* %GLYRE_SKIP: To not add this item to the results.

* %GLYRE_OK: To add this item to the results and continue happily.

* %GLYRE_STOP_POST: To stop right now and return the results. The current element will be added.

* %GLYRE_STOP_PRE: To stop right now and return the results. The current element will NOT be added.
*
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_dlcallback(GlyrQuery * settings, DL_callback dl_cb, void * userp);

/**
* glyr_opt_type:
* @s: The GlyrQuery settings struct to store this option in.
* @type: The type of metadata you want to get.
*
* Example: %GLYR_GET_COVERART
*
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_type(GlyrQuery * s, GLYR_GET_TYPE type);

/**
* glyr_opt_artist:
* @s: The GlyrQuery settings struct to store this option in.
* @artist: The artist you want to search for, %NULL and "" is not valid.
*
* This is needed for all types of metadata.
* Libglyr keeps a copy of this string internally.  
*
* <note>
* <para>
* libglyr applies some basic normalization, like " artistX feat. artistY" -> "artistX"
* </para>
* </note>
*
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_artist(GlyrQuery * s, char * artist);

/**
* glyr_opt_album:
* @s: The GlyrQuery settings struct to store this option in.
* @album: The album you want to search for, %NULL and "" is not valid.
*
* This field is required for the following types:
* <itemizedlist>
* <listitem>
* <para>
* %GLYR_GET_COVERART
* </para>
* </listitem>
* <listitem>
* <para>
* %GLYR_GET_ALBUM_REVIEW
* </para>
* </listitem>
* <listitem>
* <para>
* %GLYR_GET_TRACKLIST
* </para>
* </listitem>
* </itemizedlist>
*
* Optional for the following types:
* <itemizedlist>
* <listitem>
* <para>
* %GLYR_GET_RELATIONS
* </para>
* </listitem>
* <listitem>
* <para>
* %GLYR_GET_TAGS
* </para>
* </listitem>
* </itemizedlist>
*
* Libglyr keeps a copy of this string internally.
*
* <note>
* <para>
* libglyr applies some basic normalization, like " CoOl_album CD01 (20.7)" -> "cool_album"
* </para>
* </note>
*
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_album(GlyrQuery * s,  char * album);

/**
* glyr_opt_title:
* @s: The GlyrQuery settings struct to store this option in.
* @title: The album you want to search for, %NULL and "" is not valid.
*
* This field is required for the following types:
* <itemizedlist>
* <listitem>
* <para>
* %GLYR_GET_LYRICS
* </para>
* </listitem>
* <listitem>
* <para>
* %GLYR_GET_SIMILIAR_SONGS
* </para>
* </listitem>
* </itemizedlist>
*
* Optional for the following types:
* <itemizedlist>
* <listitem>
* <para>
* %GLYR_GET_RELATIONS
* </para>
* </listitem>
* <listitem>
* <para>
* %GLYR_GET_TAGS
* </para>
* </listitem>
* </itemizedlist>
*
* Libglyr keeps a copy of this string internally.
*
* <note>
* <para>
* libglyr applies some basic normalization, like "Songtitle (blahblah remix)" -> "Songtitle"
* </para>
* </note>
*
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_title(GlyrQuery * s,  char * title);

/**
* glyr_opt_img_minsize:
* @s: The GlyrQuery settings struct to store this option in.
* @size: The minimum size in pixels an image may have, assuming it to be quadratic 
*
* <note>
* <para>
* This is only taken as a hint, returned images are not necessarily higher than this size, but should be around it.
* </para>
* </note>
* 
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_img_minsize(GlyrQuery * s, int size);

/**
* glyr_opt_img_maxsize:
* @s: The GlyrQuery settings struct to store this option in.
* @size: The maxmimum size in pixels an image may have, assuming it to be quadratic 
*
* <note>
* <para>
* This is only taken as a hint, returned images are not necessarily below this size, but should be around it.
* </para>
* </note>
* 
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_img_maxsize(GlyrQuery * s, int size);

/**
* glyr_opt_parallel:
* @s: The GlyrQuery settings struct to store this option in.
* @parallel_jobs: The number of providers that are queried in parallel.
*
* A value of 0 lets libglyr chooses this value itself. This is the default.
* 
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_parallel(GlyrQuery * s, unsigned long parallel_jobs);

/**
* glyr_opt_timeout:
* @s: The GlyrQuery settings struct to store this option in.
* @timeout: Maximum number of seconds to wait before canceling a download.
*
* Default is 20 seconds
* 
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_timeout(GlyrQuery * s, unsigned long timeout);

/**
* glyr_opt_redirects:
* @s: The GlyrQuery settings struct to store this option in.
* @redirects: Maximum number of redirects before canceling a download.
*
* A value of 0 is allowed but may break some plugins.
* 
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_redirects(GlyrQuery * s, unsigned long redirects);

/**
* glyr_opt_useragent:
* @s: The GlyrQuery settings struct to store this option in.
* @useragent: A string that is used as useragent in HTTP requests.
*
* Some providers require an valid useragent, an empty string might break these therefore.
* 
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_useragent(GlyrQuery * s, const char * useragent);


/**
* glyr_opt_lang:
* @s: The GlyrQuery settings struct to store this option in.
* @langcode: An ISO 639-1 language code.
*
* Some providers offer localized content, or content only being available in certain countries.
* Examples are: last.fm, amazon and google.
* The language is given in ISO 639-1 codes like 'de' or 'en'.
* Alternatively you can set it to 'auto', which will cause libglyr to guess your language by your locale. 
* "auto" is the default behavior.
* 
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_lang(GlyrQuery * s, char * langcode);

/**
* glyr_opt_lang_aware_only:
* @s: The GlyrQuery settings struct to store this option in.
* @lang_aware_only: Boolean, set to true if you want language specific providers only.
*
* Note: Not for all types of metadata there may be localized content, and only fetchers, that provide text items are affected by this setting.
* The special provider 'local' is an exception here, it is queried, but delievers only language specific content too.
*   
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_lang_aware_only(GlyrQuery * s, bool lang_aware_only);

/**
* glyr_opt_number:
* @s: The GlyrQuery settings struct to store this option in.
* @num: Maximum number of items to get or 0 
*
* The maximum number of items to get in a glyr_get(), resulting number of items may be below @num but not higher.
* A value of 0 causes libglyr to search till infinity.
* Default is 1.
*
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_number(GlyrQuery * s, unsigned int num);

/**
* glyr_opt_verbosity:
* @s: The GlyrQuery settings struct to store this option in.
* @level: Define how verbose the library is.
*
* The verbosity level that is used by libglyr:
* <itemizedlist>
* <listitem>
* <para>
* 0: No output, but fatal errors.
* </para>
* </listitem>
* <listitem>
* <para>
* 1: Basic warnings.
* </para>
* </listitem>
* <listitem>
* <para>
* 2: Normal informal output
* </para>
* </listitem>
* <listitem>
* <para>
* 3: Basic debug output
* </para>
* </listitem>
* <listitem>
* <para>
* 4: Full debug output
* </para>
* </listitem>
* </itemizedlist>
*
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_verbosity(GlyrQuery * s, unsigned int level);

/**
* glyr_opt_from:
* @s: The GlyrQuery settings struct to store this option in.
* @from: A comma separated list of provider names. 
*
*
* Tell libglyr where you want your metadata want from.
* You can get a full list of providers for each getter by running @glyrc @-L
* The string you can pass here looks like this example for _cover_:
* <informalexample>
* <programlisting>
* "lastfm;google"
* </programlisting>
* </informalexample>
*
* This would query to everybody's surprise"lastfm" and "google"
* Alternatively you may use the string "all" in it:
* <informalexample>
* <programlisting>
* "all;-lastfm;"
* </programlisting>
* </informalexample>
* 
* All providers except "lastfm" (therefore the '-') are used, a '+' is also allowed, which does plain nothing.
* By default all built-in providers are used.
* You can access the providernames by calling glyr_info_get()
*
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_from(GlyrQuery * s, const char * from);

/**
* glyr_opt_plugmax:
* @s: The GlyrQuery settings struct to store this option in.
* @plugmax: Maximum number of items a single provider may retrieve.
*
* Restricts providers to retrieve at max. @plugmax items, you might use this to get results
* over several providers when glyr_opt_number() is set to something higher than 1.
*
* May be removed in future releases.
*
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_plugmax(GlyrQuery * s, int plugmax);

/**
* glyr_opt_allowed_formats:
* @s: The GlyrQuery settings struct to store this option in.
* @formats: A commaseperated list of allowed formats.
*
* Restricts providers to retrieve at max. @plugmax items, you might use this to get results
* over several providers when glyr_opt_number() is set to something higher than 1.
*
* For the getters %GLYR_GET_COVERART and %GLYR_GET_ARTIST_PHOTOS only.
* The allowed formats for images, in a comma separated list.
* Examples:
* <itemizedlist>
* <listitem>
* <para>
* "png;jpeg"
* </para>
* </listitem>
* <listitem>
* <para>
* "png;jpeg;tiff;jpg;" (default) 
* </para>
* </listitem>
* </itemizedlist>
*
* <note>
* <para>
* 'jpeg' *and* 'jpg' because some websites return strange mimetypes (should be 'jpeg' only)
* </para>
* </note>
*
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_allowed_formats(GlyrQuery * s, const char * formats);

/**
* glyr_opt_download:
* @s: The GlyrQuery settings struct to store this option in.
* @download: Wether to downlaod images or just to return the found URL.
*
* Imageproviders only return URLs, by default libglyr downloads these and
* gives you the cache. By settings glyr_opt_download() to #FALSE you tell
* libglyr that you want only the URLs (in a searchengine like fashion)
*
* An check for valid images is done however.
*
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_download(GlyrQuery * s, bool download);

/**
* glyr_opt_fuzzyness:
* @s: The GlyrQuery settings struct to store this option in.
* @fuzz: Maximal Levenshtein-distance tolerance may have, see below.
*
* libglyr features fuzzy matching to enhance search results.
* Look at the string "Equilibrium" and the accidentally mistyped version "Aquillibriu":
* Those strings will be compares using the "Levenshtein distance" (http://en.wikipedia.org/wiki/Levenshtein_distance) which basically counts
* the number of insert, substitute and delete operations to transform Equilibrium"" into "Aquillibriu".
* The distance in this case is 3 since three edit-operations are needed (one insert, substitute and deletion)
*
* The fuzziness parameter is the maximum distance two strings may have to match.
* A high distance (like about 10) matches even badly mistyped strings, but also introduces bad results.
* Low settings however will omit some good results.
*
* The default values is currently 4.
* To be more secure some correction is applied:
*
* Examples:
* <itemizedlist>
* <listitem>
* <para>
* artist:Adele - album:19 
* </para>
* </listitem>
* <listitem>
* <para>
* artist:Adele - album:21 
* </para>
* </listitem>
* <listitem>
* <para>
* lv-distance = 2 which is <= 4
* </para>
* </listitem>
* <listitem>
* <para>
* But since the lv-distance is the same as the length "21" it won't match.
* </para>
* </listitem>
* </itemizedlist>
*
* The easiest way to prevent this though, is to properly tag your music. (http://musicbrainz.org/doc/MusicBrainz_Picard).
*
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_fuzzyness(GlyrQuery * s, int fuzz);

/**
* glyr_opt_qsratio:
* @s: The GlyrQuery settings struct to store this option in.
* @ratio: A float, in the range [0.0..1.0] specifying the ratio between quality and speed.
*
* 0.00 means highest speed, querying fast providers first.
* 1.00 Takes possibly longer, but should deliver best results.
* 0.85 is the current default value.
*
* All other values, smaller 0.0, greater 1.0 are clamped to [0.0..1.0]
*
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_qsratio(GlyrQuery * s, float ratio);

/**
* glyr_opt_proxy:
* @s: The GlyrQuery settings struct to store this option in.
* @proxystring: The proxy to use, see below for the notation.
*
* The proxy to use, if any.
* It is passed in the form: [protocol://][user:pass@]yourproxy.domain[:port]
* Example:
* <itemizedlist>
* <listitem>
* <para>
* Proxy.fh-hof.de:3128
* </para>
* </listitem>
* <listitem>
* <para>
* http://hman:rootroot @ godserve.com:666
* </para>
* </listitem>
* </itemizedlist>
*
* The environment variables http_proxy, ftp_proxy, all_proxy are respected, but are overwritten by this.
*
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_proxy(GlyrQuery * s, const char * proxystring);


/**
* glyr_opt_force_utf8:
* @s: The GlyrQuery settings struct to store this option in.
* @force_utf8: To force, or not to force.
*
* For textitems only.
* Some providers (like metrolyrics) might return text with strange encodings,
* that can not be converted to regular UTF8, but might return a subset of UTF8.
* This options forces libglyr to prohibit those.
*
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_force_utf8(GlyrQuery * s, bool force_utf8);

/**
* glyr_opt_lookup_db:
* @s: The GlyrQuery settings struct to store this option in.
* @db: a GlyrDatabase object.
*
* Bind the previosly created @db to the query @s.
* By doing this you add a new 'local' provider, 
* that is queried before everything else and may speed up
* things heavily.
*
* You can either query it exclusively or disable it completely:
*
* Enable exclusiv:
* <informalexample>
* <programlisting>
* glyr_opt_from(s,"local");
* </programlisting>
* Disable:
* </informalexample>
* <informalexample>
* glyr_opt_from(s,"all;-local");
* <programlisting>
* </programlisting>
* </informalexample>
*
*
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_lookup_db(GlyrQuery * s, GlyrDatabase * db);

/**
* glyr_opt_db_autowrite:
* @s: The GlyrQuery settings struct to store this option in.
* @write_to_db: true, to write this to the database automatically 
*
* If a database is specified via glyr_opt_lookup_db you can choose
* to automatically save newly found items to the database.
* They will be looked up from there if you search for it again.
* 
* Returns: an error ID
*/
GLYR_ERROR glyr_opt_db_autowrite(GlyrQuery * s, bool write_to_db);


/**
* glyr_opt_db_autoread:
* @s: The GlyrQuery settings struct to store this option in.
* @read_from_db: Boolean, true for DB lookup while searching
*
* If set to true libglyr will lookup the database previously given by glyr_opt_lookup_db()
* during searching in the web. If there's item that already seems to be in the DB it gets not 
* mixed into the results. 
 * <note>
 * <para>
 * This does not influence the usage of the DB as local provider!
 * Use glyr_opt_from() with "all;-local" to disable it.
 * </para>
 * </note>
*
* Returns: an error ID
**/
GLYR_ERROR glyr_opt_db_autoread(GlyrQuery * s, bool read_from_db);

/**
* glyr_download:
* @url: A valid url, for example returned by libglyr
* @s: A settings struct managing timeout, useragent and redirects. 
*
* Downloads the data pointed to by @url and caches in it a GlyrMemCache, which is returned to you. 
* Use glyr_cache_free() to free it after use.
*
* Returns: A GlyrMemCache containing the data, or %NULL on failure, use verbose output to find out why.
*/
GlyrMemCache * glyr_download(const char * url, GlyrQuery * s);

/**
* glyr_strerror:
* @ID: a member of the %GLYR_ERROR enum. 
*
* Gets a descriptive message from an error ID.
*
* Returns: a descriptive nullterminated string, do <emphasis>NOT</emphasis> pass to free
*/
const char * glyr_strerror(GLYR_ERROR ID);

/**
* glyr_version:
*
* Returns: the current version string. Example below.
* 
* Version 0.4 (Larcenous Locust [dev]) of [May 20 2011] compiled at [19:12:37]
*
* Retunrs a nullterminated string, do <emphasis>NOT</emphasis> pass it to free!
*/
const char * glyr_version(void);


/**
* glyr_info_get: 
*
* get information about existing Fetcher and Source
* A Doubly linked list of Fetcher is returned, each having a field 'head',
* being a pointer to a doubly linked list of GlyrSourceInfos
*
* It is best understood by an example:
* <example>
* <title>Using GlyrFetcherInfo:</title>
* <programlisting>
* static void visualize_from_options(void)
* {
*     GlyrFetcherInfo * info = glyr_info_get();
*     if(info != NULL)
*     {
*         for(GlyrFetcherInfo * elem0 = info; elem0; elem0 = elem0->next)
*         {
*             printf("%s\n",elem0->name);
*             for(GlyrSourceInfo * elem1 = elem0->head; elem1; elem1 = elem1->next)
*             {
*                 printf("  [%c] %s\n",elem1->key,elem1->name);
*             }
*             printf("\n");
*         }
*    }
*    glyr_info_free(info);
* }
* </programlisting>
* </example>
*
* Returns: A newly allocated GlyrFetcherInfo structure, you can iterate over.
*/
GlyrFetcherInfo * glyr_info_get(void);

/**
* glyr_info_free:
* @info: The return value of glyr_info_get()
*
* Free the return value of glyr_info_get() pointed to by @info
*/
void glyr_info_free(GlyrFetcherInfo * info);

/**
* glyr_data_type_to_string:
* @type: a member of the %GLYR_DATA_TYPE enum, %GLYR_TYPE_COVERART_PRI for example
*
* Converts a type to a string.
*
* Returns: a statically allocated string, do not free
*/
const char * glyr_data_type_to_string(GLYR_DATA_TYPE type);

/**
* glyr_get_type_to_string:
* @type: a member of the %GLYR_GET_TYPE enum, %GLYR_GET_COVERART for example
* 
* Converts a get type to a string (GLYR_GET_COVERART => "cover")
* You must not modify the string or daemons will come to you at night!
*
* Returns: a statically allocated string, do not free nor modify
*/
const char * glyr_get_type_to_string(GLYR_GET_TYPE type);

/**
* glyr_md5sum_to_string:
* @md5sum: a md5sum (from a cache)
* 
* Convert a md5sum (raw data) to a human readable representation.
* String consists only of [0-9] and [a-f].
*
* Returns: a newly allocated string, 32 chars long.
*/
char * glyr_md5sum_to_string(unsigned char * md5sum);

/** 
* glyr_string_to_md5sum:
* @string: The string containing a human readable checksum (lowercase)
* @md5sum: A at lease 16 byte sized buffer of unsigned chars
*
* Convert a string to a raw-data md5sum.
* Must be a 32 char long string only containing [0-9] and [a-f]
* The new checksum is written to m5sum, which must be a buffer with 
* a size >= 16 bytes.
*/
void glyr_string_to_md5sum(const char * string, unsigned char * md5sum);

#ifdef _cplusplus
}
#endif

#endif
