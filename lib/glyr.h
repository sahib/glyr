#ifndef GLYR_H
#define GLYR_H

// va_list for Gly_setopt()
#include <stdarg.h>

// all structs used by glyr are here
#include "types.h"

enum GLYR_ERROR
{
    GLYRE_OK,           // everything is fine
    GLYRE_BAD_OPTION,   // you passed a bad option to Gly_setopt()
    GLYRE_BAD_VALUE,    // Invalid value in va_list
    GLYRE_EMPTY_STRUCT, // you passed an empty struct to Gly_setopt()
    GLYRE_NO_PROVIDER,  // setttings->provider == NULL
    GLYRE_UNKNOWN_GET   // settings->type is not valid
};
#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*callback_t) (void *user_data, const char *other_data);
void anvoke(callback_t callback, void *user_data, const char *other_data);

	// the actual main of glyr
	GlyCacheList * Gly_get(GlyQuery * settings, int * error);

	// Initialize settings struct to sane defaults
	void Gly_init_query(GlyQuery * glyrs);

	// free all memory that may been still allocated in struct
	void Gly_destroy_query(GlyQuery * sets);

	// free results
	void Gly_free_list(GlyCacheList * lst);
	void Gly_free_cache(GlyMemCache * c);
	void Gly_push_to_list(GlyCacheList * l, GlyMemCache * c);
	GlyMemCache * Gly_clist_at(GlyCacheList * clist, int iter);
	GlyMemCache * Gly_new_cache(void);

	/* Gly_opt_* methods */
	int GlyOpt_dlcallback(GlyQuery * settings, void (*dl_cb)(GlyMemCache *, GlyQuery *), void * userp);
	int GlyOpt_type(GlyQuery * s, int type);
	int GlyOpt_artist(GlyQuery * s, char * artist);
	int GlyOpt_album(GlyQuery * s,  char * album);
	int GlyOpt_title(GlyQuery * s,  char * title);
	int GlyOpt_cmaxsize(GlyQuery * s, int size);
	int GlyOpt_cminsize(GlyQuery * s, int size);
	int GlyOpt_parallel(GlyQuery * s, unsigned long val);
	int GlyOpt_timeout(GlyQuery * s, unsigned long val);
	int GlyOpt_redirects(GlyQuery * s, unsigned long val);
	int GlyOpt_lang(GlyQuery * s, char * langcode);
	int GlyOpt_number(GlyQuery * s, unsigned int num);
	int GlyOpt_verbosity(GlyQuery * s, unsigned int level);
	int GlyOpt_infoat(GlyQuery * s, int at, const char * value);
	int GlyOpt_from(GlyQuery * s, const char * from);
	int GlyOpt_color(GlyQuery * s, bool iLikeColorInMyLife);
	int GlyOpt_plugmax(GlyQuery * s, int plugmax);
	int GlyOpt_download(GlyQuery * s, bool download);

	// return library version
	const char * Gly_version(void);

	// print method that works with verbosity
	int Gly_message(int v, GlyQuery * s, FILE * stream, const char * fmt, ...);

	// get information about available plugins
	GlyPlugin * Gly_get_provider_by_id(int ID);

	// write binary file, this is for use in language bindings mainly, which partly can't easily write them themself
	int Gly_write_binary_file(const char * path, GlyMemCache * data, const char * save_dir, const char * type, GlyQuery *s);

#ifdef _cplusplus
}
#endif

#endif
