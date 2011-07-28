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

#include <errno.h>
#include <curl/multi.h>

#include "stringlib.h"
#include "core.h"

/* Checkumming */
#include "md5.h"

/* Get user agent string */
#include "config.h"

/*--------------------------------------------------------*/

// This is not a nice way. I know.
#define remove_color( string, color )             \
	if(string && color) {                     \
		char * x = strreplace(string,color,NULL); \
		free(string); string = x;}                \

int glyr_message(int v, GlyQuery * s, FILE * stream, const char * fmt, ...)
{
	int r = -1;
	if(s || v == -1)
	{
		if(v == -1 || v <= s->verbosity)
		{
			va_list param;
			char * tmp = NULL;
			if(stream && fmt)
			{
				va_start(param,fmt);
				r = x_vasprintf (&tmp,fmt,param);
				va_end(param);

				if(r != -1 && tmp)
				{

					// Remove any color if desired
					// This feels hackish.. but hey. It's just output.
#ifdef USE_COLOR
					if(s && !s->color_output)
					{
						remove_color(tmp,C_B);
						remove_color(tmp,C_M);
						remove_color(tmp,C_C);
						remove_color(tmp,C_R);
						remove_color(tmp,C_G);
						remove_color(tmp,C_Y);
						remove_color(tmp,C_ );
					}
#endif
					r = fprintf(stream,"%s%s",(v>2)?C_B"DEBUG: "C_:"",tmp);
					free(tmp);
					tmp = NULL;
				}
			}
		}
	}
	return r;
}

#undef remove_color

/*--------------------------------------------------------*/

GlyMemCache * DL_copy(GlyMemCache * src)
{
	GlyMemCache * dest = NULL;
	if(src)
	{
		dest = DL_init();
		if(dest)
		{
			if(src->data)
			{
				dest->data = calloc(src->size+1,sizeof(char));
				if(!dest->data)
				{
					glyr_message(-1,NULL,stderr,"fatal: Allocation of cachecopy failed in DL_copy()\n");
					return NULL;
				}
				memcpy(dest->data,src->data,src->size);
			}
			if(src->dsrc)
			{
				dest->dsrc = strdup(src->dsrc);
			}
			if(src->prov)
			{
				dest->prov = strdup(src->prov);
			}
			dest->size = src->size;
			dest->duration = src->duration;
			dest->error = src->error;
			dest->is_image = src->is_image;
			dest->type = src->type;

			memcpy(dest->md5sum,src->md5sum,16);
		}
	}
	return dest;
}

/*--------------------------------------------------------*/

// cache incoming data in a GlyMemCache
static size_t DL_buffer(void *puffer, size_t size, size_t nmemb, void *cache)
{
	size_t realsize = size * nmemb;
	struct GlyMemCache *mem = (struct GlyMemCache *)cache;

	mem->data = realloc(mem->data, mem->size + realsize + 1);
	if (mem->data)
	{
		memcpy(&(mem->data[mem->size]), puffer, realsize);
		mem->size += realsize;
		mem->data[mem->size] = 0;

		if(mem->dsrc && strstr(mem->data,mem->dsrc))
			return 0;
	}
	else
	{
		glyr_message(-1,NULL,stderr,"Caching failed: Out of memory.\n");
		glyr_message(-1,NULL,stderr,"Did you perhaps try to load a 4,7GB iso into your RAM?\n");
	}
	return realsize;
}

/*--------------------------------------------------------*/

// cleanup internal buffer if no longer used
void DL_free(GlyMemCache *cache)
{
	if(cache)
	{
		if(cache->size && cache->data)
		{
			free(cache->data);
			cache->data = NULL;
		}
		if(cache->dsrc)
		{
			free(cache->dsrc);
			cache->dsrc = NULL;
		}

		if(cache->prov)
		{
			free(cache->prov);
			cache->prov = NULL;
		}

		cache->size = 0;
		cache->type = TYPE_NOIDEA;

		free(cache);
		cache = NULL;
	}
}

/*--------------------------------------------------------*/

// Use this to init the internal buffer
GlyMemCache* DL_init(void)
{
	GlyMemCache *cache = malloc(sizeof(GlyMemCache));

	if(cache)
	{
		cache->size     = 0;
		cache->data     = NULL;
		cache->dsrc     = NULL;
		cache->error    = ALL_OK;
		cache->type     = TYPE_NOIDEA;
		cache->duration = 0;
		cache->is_image = false;
		cache->prov     = NULL;
		cache->next     = NULL;
		cache->prev     = NULL;
		memset(cache->md5sum,0,16);
	}
	else
	{
		glyr_message(-1,NULL,stderr,"Warning: empty dlcache. Might be noting serious.\n");
	}
	return cache;
}

/*--------------------------------------------------------*/

GlyCacheList * DL_new_lst(void)
{
	GlyCacheList * c = calloc(1,sizeof(GlyCacheList));
	if(c != NULL)
	{
		c->size = 0;
		c->usersig = GLYRE_OK;
		c->list = calloc(1,sizeof(GlyMemCache*));
		if(!c->list)
		{
			glyr_message(-1,NULL,stderr,"calloc() returned NULL!");
		}
	}
	else
	{
		glyr_message(-1,NULL,stderr,"calloc() returned NULL!");
	}

	return c;
}

/*--------------------------------------------------------*/

void DL_add_to_list(GlyCacheList * l, GlyMemCache * c)
{
	if(l && c)
	{
		l->list = realloc(l->list, sizeof(GlyMemCache *) * (l->size+2));
		l->list[  l->size] = c;
		l->list[++l->size] = NULL;
	}
}

/*--------------------------------------------------------*/

void DL_push_sublist(GlyCacheList * l, GlyCacheList * s)
{
	if(l && s)
	{
		size_t i;
		for(i = 0; i < s->size; i++)
		{
			DL_add_to_list(l,s->list[i]);
		}
	}
}

/*--------------------------------------------------------*/

// Free list itself and all caches stored in there
void DL_free_lst(GlyCacheList * c)
{
	if(c)
	{
		size_t i;
		for(i = 0; i < c->size ; i++)
			DL_free(c->list[i]);

		DL_free_container(c);
	}
}

/*--------------------------------------------------------*/

// only free list, caches are left intact
void DL_free_container(GlyCacheList * c)
{
	if(c!=NULL)
	{
		if(c->list)
			free(c->list);

		memset(c,0,sizeof(GlyCacheList));
		free(c);
	}
}

/*--------------------------------------------------------*/

// Splits http_proxy to libcurl conform represantation
static bool proxy_to_curl(GlyQuery * q, char ** userpwd, char ** server)
{
	if(q && userpwd && server)
	{
		if(q->proxy != NULL)
		{
			char * ddot = strchr(q->proxy,':');
			char * asgn = strchr(q->proxy,'@');

			if(ddot == NULL || asgn < ddot)
			{
				*server  = strdup(q->proxy);
				*userpwd = NULL;
				return true;
			}
			else
			{
				size_t len = strlen(q->proxy);
				char * protocol = strstr(q->proxy,"://");

				if(protocol == NULL)
				{
					protocol = (char*)q->proxy;
				}
				else
				{
					protocol += 3;
				}

				*userpwd = strndup(protocol,asgn-protocol);
				*server  = strndup(asgn+1,protocol+len-asgn);
				return true;
			}
		}
	}
	return false;
}

/*--------------------------------------------------------*/

// Init an easyhandler with all relevant options
static void DL_setopt(CURL *eh, GlyMemCache * cache, const char * url, GlyQuery * s, void * magic_private_ptr, long timeout)
{
	if(!s) return;

	// Set options (see 'man curl_easy_setopt')
	curl_easy_setopt(eh, CURLOPT_TIMEOUT, timeout);
	curl_easy_setopt(eh, CURLOPT_NOSIGNAL, 1L);

	// last.fm and discogs require an useragent (wokrs without too)
	curl_easy_setopt(eh, CURLOPT_USERAGENT, "liblyr ("glyr_VERSION_NAME")");
	curl_easy_setopt(eh, CURLOPT_HEADER, 0L);

	// Pass vars to curl
	curl_easy_setopt(eh, CURLOPT_URL, url);
	curl_easy_setopt(eh, CURLOPT_PRIVATE, magic_private_ptr);
	curl_easy_setopt(eh, CURLOPT_VERBOSE, (s->verbosity == 4));
	curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, DL_buffer);
	curl_easy_setopt(eh, CURLOPT_WRITEDATA, (void *)cache);

	// amazon plugin requires redirects
	curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(eh, CURLOPT_MAXREDIRS, s->redirects);

	// Do not download 404 pages
	curl_easy_setopt(eh, CURLOPT_FAILONERROR, 1L);

	// Set proxy to use
	if(s->proxy)
	{
		char * userpwd;
		char * server;
		proxy_to_curl(s,&userpwd,&server);

		if(server != NULL)
		{
			curl_easy_setopt(eh, CURLOPT_PROXY,server);
			free(server);
		}
		else
		{
			glyr_message(-1,NULL,stderr,"warning: error while parsing proxy string.\n");
		}

		if(userpwd != NULL)
		{
			curl_easy_setopt(eh, CURLOPT_PROXYUSERPWD,userpwd);
			free(userpwd);
		}
	}

	// Discogs requires gzip compression
	curl_easy_setopt(eh, CURLOPT_ENCODING,"gzip");

	// Don't save cookies - I had some quite embarassing moments
	// when amazon's startpage showed me "Hooray for Boobies",
	// because I searched for the Bloodhoundgang album... 
	// (because I have it already of course! ;-))
	curl_easy_setopt(eh, CURLOPT_COOKIEJAR ,"");
}

/*--------------------------------------------------------*/
// return a memcache only with error field set (for error report)
GlyMemCache * DL_error(int eid)
{
	GlyMemCache * ec = DL_init();
	if(ec != NULL)
	{
		ec->error = eid;
	}
	return ec;
}

/*--------------------------------------------------------*/

bool continue_search(int iter, GlyQuery * s)
{
	if(s != NULL)
	{
/*
		if((iter + s->itemctr) < s->number && (iter < s->plugmax || s->plugmax == -1))
		{
			glyr_message(4,s,stderr,"\ncontinue! iter=%d && s->number %d && plugmax=%d && items=%d\n",iter,s->number,s->plugmax,s->itemctr);
			return true;
		}
	}
	glyr_message(4,s,stderr,"\nSTOP! iter=%d && s->number %d && plugmax=%d && items=%d\n",iter,s->number,s->plugmax,s->itemctr);
*/
	}
	return true;
}

/*--------------------------------------------------------*/
// Bad data checker mehods:
/*--------------------------------------------------------*/


int flag_lint(GlyCacheList * result, GlyQuery * s)
{
	// As author of rmlint I know that there are better ways
	// to do fast duplicate finding... but well, it works ;-)
	if(!result || s->duplcheck == false)
		return 0;

	size_t i = 0, dp = 0;
	for(i = 0; i < result->size; i++)
	{
		size_t j = 0;
		if(result->list[i]->error == ALL_OK)
		{
			for(j = 0; j < result->size; j++)
			{
				if(i != j && result->list[i]->error == ALL_OK)
				{
					if(result->list[i]->size == result->list[j]->size)
					{
						/* Use by default checksums, lotsa faster */
#if !CALC_MD5SUMS
						if(!memcmp(result->list[i]->data,result->list[j]->data,result->list[i]->size))
						{
							result->list[j]->error = DOUBLE_ITEM;
							dp++;
						}
#else
						if(!memcmp(result->list[i]->md5sum,result->list[j]->md5sum,16))
						{
							result->list[j]->error = DOUBLE_ITEM;
							dp++;
						}
#endif
					}
				}
			}
		}
	}

	if(dp != 0)
	{
		const char * plural = (dp < 2) ? " " : "s ";
		glyr_message(2,s,stderr,C_"- Ignoring %d item%sthat occure%stwice.\n",dp,plural,plural);
	}
	return dp;
}

/*--------------------------------------------------------*/

int flag_invalid_format(GlyCacheList * result, GlyQuery * s)
{
	int c = 0;
	size_t i = 0;
	for(i = 0; i < result->size; i++)
	{
		if(result->list[i]->error != ALL_OK)
			continue;

		char * data = result->list[i]->data;
		size_t dlen = strlen(data);

		if(data != NULL)
		{
			// Search for '.' marking seperator from name
			char * dot_ptr = strrstr_len(data,(char*)".",dlen);
			if(dot_ptr != NULL)
			{
				// Copy format spec
				char * c_format = copy_value(dot_ptr+1,data+dlen);
				if(c_format != NULL)
				{
					ascii_strdown_modify(c_format);
					char  * f = NULL;
					size_t offset = 0, kick_me = false, flen = strlen(s->formats);
					while(!kick_me && (f = get_next_word(s->formats,";",&offset,flen)))
					{
						if(!strcmp(f,c_format) || !strcmp(f,"all"))
						{
							kick_me = true;
						}
						free(f);
						f = NULL;

						if(kick_me) break;
					}

					if(!kick_me)
					{
						result->list[i]->error = BAD_FORMAT;
						c++;
					}
				}
				free(c_format);
			}
			else
			{
				result->list[i]->error = BAD_FORMAT;
				c++;
			}
		}
	}
	if(c != 0)
	{
		glyr_message(2,s,stderr,C_"- Ignoring %d images with unknown format.\n",c);
	}
	return c;
}

/*--------------------------------------------------------*/

int flag_blacklisted_urls(GlyCacheList * result, const char ** URLblacklist, GlyQuery * s)
{
	if(s->duplcheck == false)
		return 0;

	size_t i = 0, ctr = 0;
	for(i = 0; i < result->size; i++)
	{
		if(result->list[i]->error != ALL_OK)
			continue;

		if(result->list[i]->error == 0)
		{
			size_t j = 0;
			for(j = 0; URLblacklist[j]; j++)
			{
				if(result->list[j]->error == 0)
				{
					if(!strcmp(result->list[i]->data,URLblacklist[j]))
					{
						result->list[i]->error = BLACKLISTED;
						ctr++;
					}
				}
			}
		}
	}
	if(ctr != 0)
	{
		glyr_message(2,s,stderr,C_"- Ignoring %d blacklisted URL%s.\n\n",ctr,ctr>=1 ? " " : "s ");
	}
	return ctr;
}

/*--------------------------------------------------------*/

// Download a singe file NOT in parallel
GlyMemCache * download_single(const char* url, GlyQuery * s, const char * end)
{
	if(url==NULL)
	{
		return NULL;
	}

	CURL *curl = NULL;
	CURLcode res = 0;

	// Init handles
	curl = curl_easy_init();
	GlyMemCache * dldata = DL_init();

	// DL_buffer needs the 'end' mark.
	// As I didnt want to introduce a new struct just for this
	// I save it in ->dsrc
	if(end != NULL)
	{
		dldata->dsrc = strdup(end);
	}

	if(curl != NULL)
	{
		// Configure curl
		DL_setopt(curl,dldata,url,s,NULL,s->timeout);

		// Perform transaction
		res = curl_easy_perform(curl);

		// Better check again
		if(res != CURLE_OK && res != CURLE_WRITE_ERROR)
		{
			glyr_message(-1,NULL,stderr,C_"\ncurl-error: %s [E:%d]\n", curl_easy_strerror(res),res);
			DL_free(dldata);
			dldata = NULL;
		}
		else
		{
			// Set the source URL
			if(dldata->dsrc != NULL)
				free(dldata->dsrc);

			dldata->dsrc = strdup(url);
		}
		// Handle without any use - clean up
		curl_easy_cleanup(curl);

		update_md5sum(dldata);
		return dldata;
	}

	// Free mem
	DL_free(dldata);
	return NULL;
}

/*--------------------------------------------------------*/

// Init a callback object and a curl_easy_handle
static GlyMemCache * handle_init(CURLM * cm, cb_object * capo, GlyQuery *s, long timeout)
{
	GlyMemCache * dlcache = NULL;
	if(capo && capo->url) 
	{
		/* Init handle */
		CURL *eh = curl_easy_init();

		/* Init cache */
		dlcache = DL_init();

		/*
		   if(capo && capo->plug && capo->plug->plug.endmarker){
		   dlcache->dsrc = strdup(capo->plug->plug.endmarker);
		   }
		 */

		/* Remind this handle */
		capo->handle = eh;

		/* Configure this handle */
		DL_setopt(eh, dlcache, capo->url, s,(void*)capo,timeout);

		/* Add handle to multihandle */
		curl_multi_add_handle(cm, eh);
	}
	return dlcache;
}

/*--------------------------------------------------------*/
/* ----------------- THE HEART OF GOLD ------------------ */
/*--------------------------------------------------------*/

GList * async_download(GList * url_list, GlyQuery * s, long parallel_fac, long timeout_fac, AsyncDLCB callback, void * userptr)
{
	/* Storage for result items */
	GList * item_list = NULL;	

	if(url_list != NULL && s != NULL)
	{
		/* total timeout and parallel tries */
		long abs_timeout  = ABS(timeout_fac  * s->timeout);
		long abs_parallel = ABS(parallel_fac * s->parallel);

		/* select() control */
		int M, Q, U = -1;
		long wait_time;
		fd_set ReadFDS, WriteFDS, ErrorFDS;

		/* Curl Handlers (~ container for easy handlers) */	
		CURLM   * cmHandle = curl_multi_init();
		curl_multi_setopt(cmHandle, CURLMOPT_MAXCONNECTS,abs_parallel);
		curl_multi_setopt(cmHandle, CURLMOPT_PIPELINING, 1L);

		GList * elem, * cb_list = NULL;

		/* Now create cb_objects */
		for (elem = url_list; elem; elem = elem->next) {
			cb_object * obj = calloc(1,sizeof(cb_object));
			obj->url   = g_strdup((gchar*)(elem->data));
			obj->cache = handle_init(cmHandle,obj,s,abs_timeout);
			obj->s     = s;
			cb_list    = g_list_prepend(cb_list,obj); 
		}

		while(U != 0) {
			/* Store number of handles still being transferred in U */
			CURLMcode merr;
			if((merr = curl_multi_perform(cmHandle, &U)) != CURLM_OK)
				return NULL;	

			if(U != 0) {
				/* Set up fds */
				FD_ZERO(&ReadFDS);
				FD_ZERO(&WriteFDS);
				FD_ZERO(&ErrorFDS);

				if (curl_multi_fdset(cmHandle, &ReadFDS, &WriteFDS, &ErrorFDS, &M) ||
						curl_multi_timeout(cmHandle, &wait_time)) {
					fprintf(stderr,"glyr: error while selecting stream. Might be a bug.\n");
					return NULL;
				}

				if (wait_time == -1)
					wait_time = 100;

				/* Nothing happens.. */
				if (M == -1) {
					g_usleep(wait_time * 100);
				} else {
					struct timeval Tmax;
					Tmax.tv_sec  = (wait_time/1000);
					Tmax.tv_usec = (wait_time%1000)*1000;

					/* Now block till something interesting happens with the download */
					if (select(M+1, &ReadFDS, &WriteFDS, &ErrorFDS, &Tmax) == -1) {
						glyr_message(-1,NULL,stderr, "E: select(%i <=> %li): %i: %s\n",
								M+1, wait_time, errno, strerror(errno));
						return NULL;
					}
				}
			}

			/* select() returned. There might be some fresh flesh! - Check. */
			CURLMsg * msg;
			while ((msg = curl_multi_info_read(cmHandle, &Q)))
			{
				/* That download is ready to be viewed */
				if (msg->msg == CURLMSG_DONE)
				{
					/* Easy handle of this particular DL */
					CURL *easy_handle = msg->easy_handle;

					/* Get the callback object associated with the curl handle
					 * for some odd reason curl requires a char * pointer */
					cb_object * capo;
					curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, (const char*)&capo);

					/* capo contains now the downloaded cache, ready to parse */
					if(msg->data.result == CURLE_OK && capo && capo->cache)
					{
						bool stop_download = false;
						bool add_item = true;
						GList * cb_results = NULL;

						/* Set origin */
						capo->cache->dsrc = g_strdup(capo->url);

						/* Call it if present */
						if(callback != NULL) {
							/* Add parsed results or nothing if parsed result is empty */
							cb_results = callback(capo,userptr,&stop_download,&add_item);
						}

						if(cb_results != NULL) {

							/* Fill in the source filed (dsrc) if not already done */
							for(GList * elem = cb_results; elem; elem = elem->next)
							{
								GlyMemCache * item = elem->data;
								if(item->dsrc == NULL)
								{
									/* Plugin didn't do any special download */
									item->dsrc = g_strdup(capo->url);
								}
							}
							item_list = g_list_concat(item_list,cb_results);
						
						} else if(add_item != false) {
							/* Add it as raw data */
							item_list = g_list_prepend(item_list,capo->cache);
						}
					} else {
						/* Something in this download was wrong. Tell us what. */
						char * errstring = (char*)curl_easy_strerror(msg->data.result);
						fprintf(stderr, "glyrE: Downloaderror: %s [errno:%d]\n",
								errstring ? errstring : "Unknown Error",
								msg->data.result);
					}

					/* We're done with this one.. bybebye */
					curl_multi_remove_handle(cmHandle,easy_handle);
					curl_easy_cleanup(easy_handle);
					capo->handle = NULL;
				} else {
					/* Something in the multidownloading gone wrong */
					fprintf(stderr,"glyrE: multiDL-errorcode: %d\n",msg->msg);
				}
			}
		}
		/* Free ressources */
		curl_multi_cleanup(cmHandle);

		if(cb_list != NULL) {
			for (elem = cb_list; elem; elem = elem->next) {
				cb_object * item = elem->data;
				free(item->url);
				free(item);
			}
			g_list_free(cb_list);
		}
	}
	return item_list;
} 

/*--------------------------------------------------------*/

/* The actual call to the metadata provider here, coming from the downloader, triggered by start_engine() */
GList * call_provider_callback(cb_object * capo, void * userptr, bool * stop_download, bool * add_item)
{
	GList * parsed = NULL;
	if(userptr != NULL) {
		/* Get MetaDataSource correlated to this URL */
		GHashTable * assoc = (GHashTable*)userptr;
		MetaDataSource * plugin = g_hash_table_lookup(assoc,capo->url);

		/* Cross fingers... */
		if(plugin != NULL) {
			/* Call the parserplugin */
			GlyCacheList * sub = plugin->parser(capo);

			/* Convert to a GList - will be removed later */
			if(sub != NULL) 
			{
				/* Convert to GList - replace this later */
				for(int i = 0; i < (int)sub->size; i++) {
					sub->list[i]->prov = g_strdup(plugin->name);
					parsed = g_list_prepend(parsed,sub->list[i]);
				}
				DL_free_container(sub);
			}
		} else {
			fprintf(stderr,"glyr: hashmap lookup failed. Cannot call plugin => Bug.\n");
		}
	}

	/* We replace the cache with a new one -> free the old one */
	if(capo->cache != NULL)
	{
		DL_free(capo->cache);
	}


	/* Do not add the 'raw' cache */
	if(parsed == NULL)
	{
		*add_item = false;
	}
	return parsed;
}

/*--------------------------------------------------------*/

bool provider_is_enabled(GlyQuery * s, MetaDataSource * f)
{
	/* Assume 'all we have' */
	if(s->from == NULL)
	    return true;

	bool isFound = false;

	/* split string */
	if(f->name != NULL) 
	{
		size_t len = strlen(s->from);
		size_t offset = 0;

		char * token = NULL;
		while((token = get_next_word(s->from,DEFAULT_FROM_ARGUMENT_DELIM,&offset,len)) && isFound == false)
		{
			if(token[0] == f->key || !strcasecmp(token,f->name))
				isFound = true;

			free(token);
		}
	}
	return isFound;
}

/*--------------------------------------------------------*/

GList * start_engine(GlyQuery * query, MetaDataFetcher * fetcher)
{
	GList * url_list = NULL;
	GHashTable * url_table = g_hash_table_new(g_str_hash,g_str_equal);

	/* Iterate over all sources for this fetcher  */
	for(GList * source = fetcher->provider; source != NULL; source = source->next)
	{
		MetaDataSource * item = source->data;
		if(item != NULL && provider_is_enabled(query,item))
		{
			/* get the url of this MetaDataSource */
			const gchar * lookup_url = item->get_url(query);

			/* Add this to the list */
			if(lookup_url != NULL)
			{
				/* make a sane URL out of it */ 
				const gchar * prepared = prepare_url(lookup_url,
						query->artist,
						query->album,
						query->title);

				/* add it to the hash table and relate it to the MetaDataSource */
				g_hash_table_insert(url_table,(gpointer)prepared,(gpointer)item);
				url_list = g_list_prepend(url_list,(gpointer)prepared);

				/* If the URL was dyn. allocated, we should go and free it */
				if(item->free_url == true)
				{
					g_free((gchar*)lookup_url);
				}
			}
		}
	}

	/* Now start the downloadmanager - and call the specified callback with the URL table when an item is ready */
	GList * raw_parsed = async_download(url_list,query,g_list_length(url_list)/2,1,call_provider_callback,url_table);
	GList * ready_caches = fetcher->finalize(query, raw_parsed);
	
	/* Raw data not needed anymore */
	g_list_free(raw_parsed);
	raw_parsed = NULL;

	/* Same goes for url list */
	for(GList * elem = url_list; elem; elem = elem->next)
	{
		g_free((gchar*)elem->data);
	}
	g_list_free(url_list);
	url_list = NULL;	

	if(ready_caches != NULL)
	{
		for(GList * elem = ready_caches; elem; elem = elem->next)
		{
			GlyMemCache * item = elem->data;

			/* md5sum is missing. */
			update_md5sum(item);

			/* Call the callback late, i.e. here. */
			query->callback.download(item,query);
		}
	} 
	
	g_hash_table_destroy(url_table);
	return ready_caches;
}

/*--------------------------------------------------------*/

GlyCacheList * generic_finalizer(GlyCacheList * result, GlyQuery * settings, int type)
{
	if(!result) return NULL;

	size_t i = 0;
	GlyCacheList * r_list = NULL;

	/* Ignore double items if desired */
	flag_lint(result,settings);

	for(i = 0; i < result->size; i++)
	{
		if(result->list[i]->error != ALL_OK)
			continue;

		if(!r_list) r_list = DL_new_lst();
		if(result->list[i]->type == TYPE_NOIDEA)
			result->list[i]->type = type;

		// call user defined callback
		if(settings->callback.download)
		{
			// Call the usercallback
			r_list->usersig = settings->callback.download(result->list[i],settings);
		}

		if(r_list->usersig == GLYRE_OK)
		{
			// Now make a copy of the item and add it to the list
			DL_add_to_list(r_list,DL_copy(result->list[i]));
		}
		else if(r_list->usersig == GLYRE_STOP_BY_CB)
		{
			// Break if desired.
			break;
		}
	}
	return r_list;
}

/*--------------------------------------------------------*/

void update_md5sum(GlyMemCache * c)
{
	if(c && c->data && c->size)
	{
		MD5_CTX mdContext;
		MD5Init (&mdContext);
		MD5Update(&mdContext,(unsigned char*)c->data,c->size);
		MD5Final(&mdContext);
		memcpy(c->md5sum, mdContext.digest, 16);
	}
}

/*--------------------------------------------------------*/
