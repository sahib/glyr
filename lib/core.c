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

/* Mini blacklist */
#include "blacklist.h"

/* Somehow needed to prevent some compiler warning.. */
#include <glib/gprintf.h>

/*--------------------------------------------------------*/

static int _msg(const char * fmt, va_list params)
{
	gchar * tmp_buf = NULL;
	gint written = g_vasprintf(&tmp_buf,fmt,params);

	if(written != -1 && tmp_buf != NULL)
	{
		g_printerr(tmp_buf);
		g_free(tmp_buf);
	}
	return written;
}

/*--------------------------------------------------------*/

void panic(const char * fmt, ...)
{
	va_list list;
	va_start(list,fmt);
	_msg(fmt,list);
	va_end(list);
}

/*--------------------------------------------------------*/

int glyr_message(int verbosity, GlyrQuery * s, const char * fmt, ...)
{
    gint written = 0;
    if(s != NULL || verbosity == -1)
    {
        if(verbosity == -1 || verbosity <= s->verbosity)
        {
            va_list params;
            if(fmt != NULL)
            {
                va_start(params,fmt);
		written = _msg(fmt,params);
                va_end(params);
            }
        }
    }
    return written;
}

/*--------------------------------------------------------*/

GlyrMemCache * DL_copy(GlyrMemCache * src)
{
    GlyrMemCache * new = NULL;
    if(src != NULL)
    {
	new = g_malloc0(sizeof(GlyrMemCache));
	memcpy(new,src, sizeof(GlyrMemCache));
    }
    return new;
}

/*--------------------------------------------------------*/

/* cache incoming data in a GlyrMemCache */
static size_t DL_buffer(void *puffer, size_t size, size_t nmemb, void *cache)
{
    size_t realsize = size * nmemb;
    struct GlyrMemCache *mem = (struct GlyrMemCache *)cache;

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
        glyr_message(-1,NULL,"Caching failed: Out of memory.\n");
        glyr_message(-1,NULL,"Did you perhaps try to load a 4,7GB iso into your RAM?\n");
    }
    return realsize;
}

/*--------------------------------------------------------*/

// cleanup internal buffer if no longer used
void DL_free(GlyrMemCache *cache)
{
    if(cache)
    {
        if(cache->size && cache->data)
        {
            g_free(cache->data);
            cache->data = NULL;
        }
        if(cache->dsrc)
        {
            g_free(cache->dsrc);
            cache->dsrc = NULL;
        }

        if(cache->prov)
        {
            g_free(cache->prov);
            cache->prov = NULL;
        }

        cache->size = 0;
        cache->type = TYPE_NOIDEA;

        g_free(cache->img_format);
        g_free(cache);
        cache = NULL;
    }
}

/*--------------------------------------------------------*/

// Use this to init the internal buffer
GlyrMemCache* DL_init(void)
{
    GlyrMemCache *cache = g_malloc0(sizeof(GlyrMemCache));
    memset(cache,0,sizeof(GlyrMemCache));
    cache->duration = 0;
    cache->type = TYPE_NOIDEA;
    return cache;
}

/*--------------------------------------------------------*/

// Splits http_proxy to libcurl conform represantation
static gboolean proxy_to_curl(gchar * proxystring, char ** userpwd, char ** server)
{
	if(proxystring && userpwd && server)
	{
		if(proxystring != NULL)
		{
			gchar * ddot = strchr(proxystring,':');
			gchar * asgn = strchr(proxystring,'@');

			if(ddot == NULL || asgn < ddot)
			{
				*server  = strdup(proxystring);
				*userpwd = NULL;
				return TRUE;
			}
			else
			{
				gsize len = strlen(proxystring);
				char * protocol = strstr(proxystring,"://");

				if(protocol == NULL)
				{
					protocol = (gchar*)proxystring;
				}
				else
				{
					protocol += 3;
				}

				*userpwd = g_strndup(protocol,asgn-protocol);
				*server  = g_strndup(asgn+1,protocol+len-asgn);
				return TRUE;
			}
		}
	}
	return FALSE;
}


/*--------------------------------------------------------*/

struct header_data
{
	gchar * type;
	gchar * format;
	gchar * extra;
};

/*--------------------------------------------------------*/

/* Parse header file. Get Contenttype from it and save it in the header_data struct */
gsize header_cb(void *ptr, gsize size, gsize nmemb, void *userdata)
{
	gsize bytes = size * nmemb;
	if(ptr != NULL && userdata != NULL)
	{
		/* Transform safely into string */
		gchar nulbuf[bytes + 1];
		memcpy(nulbuf,ptr,bytes);
		nulbuf[bytes] = '\0';

		/* We're only interested in the content type */
		gchar * cttp  = "Content-Type: ";
		gsize ctt_len = strlen(cttp);
		if(ctt_len < bytes && g_strncasecmp(cttp,nulbuf,ctt_len) == 0)
		{
			gchar ** content_type = g_strsplit_set(nulbuf + ctt_len," /;",0);
			if(content_type != NULL)
			{
				gsize set_at = 0;
				gchar ** elem = content_type;
				struct header_data * info = userdata;

				/* Set fields..  */
				while(elem[0] != NULL)
				{
					if(elem[0][0] != '\0')
					{
						switch(set_at)
						{
							case 0:
								g_free(info->type);
								info->type   = g_strdup(elem[0]);
								break;
							case 1:
								g_free(info->format);
								info->format = g_strdup(elem[0]);
								break;
							case 2:
								g_free(info->extra);
								info->extra  = g_strdup(elem[0]);
								break;
						}
						set_at++;
					}
					elem++;
				}
				g_strfreev(content_type);
			}
		}
	}
	return bytes;
}

/*--------------------------------------------------------*/

/* empty callback just prevent writing header to stdout */
gsize empty_cb(void * p, gsize s, gsize n, void * u)
{
	return s*n;
};

/*--------------------------------------------------------*/

static void DL_setproxy(CURL *eh, gchar * proxystring)
{
	if(proxystring != NULL)
	{
		gchar * userpwd;
		gchar * server;
		proxy_to_curl(proxystring,&userpwd,&server);

		if(server != NULL)
		{
			curl_easy_setopt(eh, CURLOPT_PROXY,server);
			g_free(server);
		}
		else
		{
			panic("glyr: Warning: Invalid proxy string.\n");
		}

		if(userpwd != NULL)
		{
			curl_easy_setopt(eh,CURLOPT_PROXYUSERPWD,userpwd);
			g_free(userpwd);
		}
	}
}

/*--------------------------------------------------------*/

static struct header_data * retrieve_content_info(gchar * url, gchar * proxystring, gchar * useragent)
{
	struct header_data * info = NULL;
	if(url != NULL)
	{
		CURL * eh = curl_easy_init();
		CURLcode rc = CURLE_OK;

		info = g_malloc0(sizeof(struct header_data));
		gchar * link_user_agent =  g_strdup_printf("%s/linkvalidator",useragent);

		curl_easy_setopt(eh, CURLOPT_TIMEOUT, 5);
		curl_easy_setopt(eh, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(eh, CURLOPT_USERAGENT, link_user_agent);
		curl_easy_setopt(eh, CURLOPT_URL,url);
		curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, TRUE);
		curl_easy_setopt(eh, CURLOPT_MAXREDIRS, 5L);
		curl_easy_setopt(eh, CURLOPT_HEADER,TRUE);

		/* Dirty hack here: Amazon bitches at me when setting NOBODY to true *
                 * But otherwise large images won't pass with other providers        * 
                 * Check domain therefore.. 
                 */
		if(strstr(url,"amazon") != NULL)
		{
			curl_easy_setopt(eh, CURLOPT_NOBODY,FALSE);
		}
		else
		{
			curl_easy_setopt(eh, CURLOPT_NOBODY,TRUE);
		}

		curl_easy_setopt(eh, CURLOPT_HEADERFUNCTION, header_cb);
		curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, empty_cb);
		curl_easy_setopt(eh, CURLOPT_WRITEHEADER, info);

		/* Set proxy, if any */
		DL_setproxy(eh, proxystring);

		/* This seemed to prevent some valid urls from passing. Strange. */
		//curl_easy_setopt(eh, CURLOPT_FAILONERROR,TRUE);

		rc = curl_easy_perform(eh);
		curl_easy_cleanup(eh);

		if(rc != CURLE_OK)
		{
			panic("- g_ping: E: %s [%d]\n",curl_easy_strerror(rc),rc);
			g_free(info);
			info = NULL;

		}
		else
		{
			/* Remove trailing newlines,carriage returns */
			chomp_breakline(info->type);
			chomp_breakline(info->format);
			chomp_breakline(info->extra);
		}

		g_free(link_user_agent);
	}
	return info;
}

/*--------------------------------------------------------*/

// Init an easyhandler with all relevant options
static void DL_setopt(CURL *eh, GlyrMemCache * cache, const char * url, GlyrQuery * s, void * magic_private_ptr, long timeout)
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
	DL_setproxy(eh,(gchar*)s->proxy);

	// Discogs requires gzip compression
	curl_easy_setopt(eh, CURLOPT_ENCODING,"gzip");

	// Don't save cookies - I had some quite embarassing moments
	// when amazon's startpage showed me "Hooray for Boobies",
	// because I searched for the Bloodhoundgang album...
	// (because I have it already of course! ;-))
	curl_easy_setopt(eh, CURLOPT_COOKIEJAR ,"");
}

/*--------------------------------------------------------*/

gboolean continue_search(gint current, GlyrQuery * s)
{
	gboolean decision = FALSE;
	if(s != NULL)
	{
		/* Take an educated guess, let the provider get more, because URLs might be wrong,
		 * as we check this later, it's good to have some more ULRs waiting for us,     *
		 * alternatively we might hit the maximum for one plugin (off by one!) 	        */
		gint buffering = (s->imagejob) ? s->number / 3 : 0;
		decision = (current + s->itemctr) < (s->number + buffering) &&
			   (current < s->plugmax || (s->plugmax == -1));

	}
	return decision;
}

/*--------------------------------------------------------*/
// Bad data checker mehods:
/*--------------------------------------------------------*/

/* Check for dupes. This does not affect the HEAD of the list, therefore no GList return */
gsize delete_dupes(GList * result, GlyrQuery * s)
{
	if(!result || g_list_length(result) < 1)
		return 0;

	/* Build new hashes, the data might have changed */
	for(GList * elem = result; elem; elem = elem->next)
	{
		update_md5sum(elem->data);
	}

	gint double_items = 0;
	for(GList * inode = result; inode; inode = inode->next)
	{
		GlyrMemCache * lval = inode->data;

		GList * jnode = result;
		while(jnode != NULL)
		{
			bool is_duplicate  = false;
			GlyrMemCache * rval = jnode->data;

			if(lval && rval && rval != lval && lval->size == rval->size)
			{
				/* Compare via checkums */
				if(CALC_MD5SUMS == false)
				{
					if(!memcmp(lval->data,rval->data,rval->size))
					{
						is_duplicate = true;
					}

				}
				else
				{
					if(!memcmp(lval->md5sum,rval->md5sum,16))
					{
						is_duplicate = true;
					}
				}

				/* Delete this element.. */
				if(is_duplicate == true)
				{
					DL_free(rval);

					/* Delete this ref from the list */
					GList * to_free = jnode;
					jnode = jnode->next;
					result = g_list_delete_link(result,to_free);

					/* Remember him.. */
					double_items++;
					continue;
				}
			}
			jnode = jnode->next;
		}
	}

	return double_items;
}

/*--------------------------------------------------------*/

// Download a singe file NOT in parallel
GlyrMemCache * download_single(const char* url, GlyrQuery * s, const char * end)
{
	if(url != NULL && is_blacklisted((gchar*)url) == false)
	{
		CURL *curl = NULL;
		CURLcode res = 0;

		/* Init handles */
		curl = curl_easy_init();
		GlyrMemCache * dldata = DL_init();

		/* DL_buffer needs the 'end' mark.
		 * As I didnt want to introduce a new struct just for this
		 * I save it in->dsrc */
		if(end != NULL)
		{
			dldata->dsrc = strdup(end);
		}

		if(curl != NULL)
		{
			/* Configure curl */
			DL_setopt(curl,dldata,url,s,NULL,s->timeout);

			/* Perform transaction */
			res = curl_easy_perform(curl);

			/* Better check again */
			if(res != CURLE_OK && res != CURLE_WRITE_ERROR)
			{
				glyr_message(3,s,"glyr: E: singledownload: %s [E:%d]\n", curl_easy_strerror(res),res);
				DL_free(dldata);
				dldata = NULL;
			}
			else
			{
				/* Set the source URL */
				if(dldata->dsrc != NULL)
				{
					g_free(dldata->dsrc);
				}

				dldata->dsrc = g_strdup(url);
			}
			/* Handle without any use - clean up */
			curl_easy_cleanup(curl);
			update_md5sum(dldata);
			return dldata;
		}

		// Free mem
		DL_free(dldata);
	}
	return NULL;
}

/*--------------------------------------------------------*/

// Init a callback object and a curl_easy_handle
static GlyrMemCache * init_async_cache(CURLM * cm, cb_object * capo, GlyrQuery *s, long timeout, gchar * endmark)
{
	GlyrMemCache * dlcache = NULL;
	if(capo && capo->url)
	{
		/* Init handle */
		CURL *eh = curl_easy_init();

		/* Init cache */
		dlcache = DL_init();

		/* Little hack, pack it in dsrc, since it isn't used at this point */
		dlcache->dsrc = g_strdup(endmark);

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

static GList * init_async_download(GList * url_list, GList * endmark_list, CURLM * cmHandle, GlyrQuery * s, int abs_timeout)
{
	GList * cb_list = NULL;
	for(GList * elem = url_list; elem; elem = elem->next)
	{
		if(is_blacklisted((gchar*)elem->data) == false)
		{
			cb_object * obj = g_malloc0(sizeof(cb_object));
			obj->s = s;
			obj->url = g_strdup((gchar*)(elem->data));
			cb_list = g_list_prepend(cb_list,obj);
			obj->consumed = FALSE;

			/* Get the endmark from the endmark list */
			gint endmark_pos = g_list_position(url_list,elem);
			GList * glist_m  = g_list_nth(endmark_list,endmark_pos);
			gchar * endmark  = (glist_m==NULL) ? NULL : glist_m->data;

			obj->cache = init_async_cache(cmHandle,obj,s,abs_timeout,endmark);
		}
	}
	return cb_list;
}

/*--------------------------------------------------------*/

static void destroy_async_download(GList * cb_list, CURLM * cmHandle, gboolean free_caches)
{
	/* Free ressources */
	curl_multi_cleanup(cmHandle);

	if(cb_list != NULL)
	{
		for (GList * elem = cb_list; elem; elem = elem->next)
		{
			cb_object * item = elem->data;
			if(item->handle != NULL)
			{
				curl_easy_cleanup(item->handle);
			}

			if(free_caches == TRUE && item->consumed == FALSE)
			{
				DL_free(item->cache);
				item->cache = NULL;
			}

			g_free(item->url);
		}
		glist_free_full(cb_list,g_free);
	}
}

/*--------------------------------------------------------*/
/* ----------------- THE HEART OF GOLD ------------------ */
/*--------------------------------------------------------*/
GList * async_download(GList * url_list, GList * endmark_list, GlyrQuery * s, long parallel_fac, long timeout_fac, AsyncDLCB asdl_callback, void * userptr, gboolean free_caches)
{
	/* Storage for result items */
	GList * item_list = NULL;

	if(url_list != NULL && s != NULL)
	{
		/* total timeout and parallel tries */
		long abs_timeout  = ABS(timeout_fac  * s->timeout);
		long abs_parallel = ABS(parallel_fac * s->parallel);

		/* select() control */
		int max_fd, queue_msg, running_handles = -1;
		long wait_time;
		fd_set ReadFDS, WriteFDS, ErrorFDS;

		/* Curl Multi Handles (~ container for easy handlers) */
		CURLM   * cmHandle = curl_multi_init();
		curl_multi_setopt(cmHandle, CURLMOPT_MAXCONNECTS,abs_parallel);
		curl_multi_setopt(cmHandle, CURLMOPT_PIPELINING, 1L);

		/* Once set to true this will terminate the download */
		gboolean terminate = FALSE;

		/* Now create cb_objects */
		GList * cb_list = init_async_download(url_list,endmark_list,cmHandle,s,abs_timeout);

		while(running_handles != 0 && terminate == FALSE)
		{
			CURLMcode merr = CURLM_CALL_MULTI_PERFORM;
			while(merr == CURLM_CALL_MULTI_PERFORM) 
			{
				merr = curl_multi_perform(cmHandle, &running_handles);
			}
			if(merr != CURLM_OK)
			{
				panic("curl_multi_perform() failed!");
				return NULL;
			}

			if(running_handles != 0)
			{
				/* Set up fds */
				FD_ZERO(&ReadFDS);
				FD_ZERO(&WriteFDS);
				FD_ZERO(&ErrorFDS);

				if (curl_multi_fdset(cmHandle, &ReadFDS, &WriteFDS, &ErrorFDS, &max_fd) ||
						curl_multi_timeout(cmHandle, &wait_time))
				{
					panic("glyr: error while selecting stream. Might be a bug.\n");
					return NULL;
				}

				if (wait_time == -1)
					wait_time = 100;

				/* Nothing happens.. */
				if (max_fd == -1)
				{
					g_usleep(wait_time * 100);
				}
				else
				{
					struct timeval Tmax;
					Tmax.tv_sec  = (wait_time/1000);
					Tmax.tv_usec = (wait_time%1000)*1000;

					/* Now block till something interesting happens with the download */
					if (select(max_fd+1, &ReadFDS, &WriteFDS, &ErrorFDS, &Tmax) == -1)
					{
						panic("glyr: E: select(%i <=> %li): %i: %s\n",max_fd+1, wait_time, errno, strerror(errno));
						return NULL;
					}
				}
			}

			/* select() returned. There might be some fresh flesh! - Check. */
			CURLMsg * msg;
			while (terminate == FALSE && (msg = curl_multi_info_read(cmHandle, &queue_msg)))
			{
				/* That download is ready to be viewed */
				if(msg->msg == CURLMSG_DONE)
				{
					/* Easy handle of this particular DL */
					CURL *easy_handle = msg->easy_handle;

					/* Get the callback object associated with the curl handle
					 * for some odd reason curl requires a char * pointer */
					cb_object * capo = NULL;
					curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, (const char*)&capo);

					/* It's useless if it's empty  */
					if(capo && capo->cache && capo->cache->data == NULL)
					{
						capo->consumed = TRUE;
						DL_free(capo->cache);
						capo->cache = NULL;
					}

					/* capo contains now the downloaded cache, ready to parse */
					if(msg->data.result == CURLE_OK && capo && capo->cache)
					{
						/* How many items from the callback will actually be added */
						gint to_add = 0;

						/* Stop download after this came in */
						bool stop_download = false;
						GList * cb_results = NULL;

						/* Set origin */
						if(capo->cache->dsrc != NULL)
						{
							g_free(capo->cache->dsrc);
						}
						capo->cache->dsrc = g_strdup(capo->url);

						/* Always update in case the callback needs it */	
						update_md5sum(capo->cache);

						/* Call it if present */
						if(asdl_callback != NULL)
						{
							/* Add parsed results or nothing if parsed result is empty */
							cb_results = asdl_callback(capo,userptr,&stop_download,&to_add);
						}

						if(cb_results != NULL)
						{
							/* Fill in the source filed (dsrc) if not already done */
							for(GList * elem = cb_results; elem; elem = elem->next)
							{
								GlyrMemCache * item = elem->data;
								if(item && item->dsrc == NULL)
								{
									/* Plugin didn't do any special download */
									item->dsrc = g_strdup(capo->url);
								}
								item_list = g_list_prepend(item_list,item);
							}
							g_list_free(cb_results);
						}
						else if(to_add != 0)
						{
							/* Add it as raw data */
							item_list = g_list_prepend(item_list,capo->cache);
						}
						else
						{
							capo->consumed = TRUE;
							DL_free(capo->cache);
							capo->cache = NULL;
						}

						/* So, shall we stop? */
						terminate = stop_download;

					}
					else
					{
						/* Something in this download was wrong. Tell us what. */
						char * errstring = (char*)curl_easy_strerror(msg->data.result);
						glyr_message(3,capo->s,"- glyr: Downloaderror: %s [errno:%d]\n",
								errstring ? errstring : "Unknown Error",
								msg->data.result);
						glyr_message(3,capo->s,"  On URL: %s\n",capo->url);

						DL_free(capo->cache);
						capo->cache = NULL;
						capo->consumed = TRUE;
					}

					/* We're done with this one.. bybebye */
					curl_multi_remove_handle(cmHandle,easy_handle);
					curl_easy_cleanup(easy_handle);
					capo->handle = NULL;
				}
				else
				{
					/* Something in the multidownloading gone wrong */
					panic("glyrE: multiDL-errorcode: %d\n",msg->msg);
				}
			}
		}
		destroy_async_download(cb_list,cmHandle,free_caches);
	}
	return item_list;
}

/*--------------------------------------------------------*/

struct wrap_retrieve_pass_data
{
	gchar * url;
	GlyrQuery * query;
};

static void * wrap_retrieve_content(gpointer data)
{
	struct header_data * head = NULL;
	if(data != NULL)
	{
		struct wrap_retrieve_pass_data * passed = data;
		GlyrQuery * query = passed->query;
		head = retrieve_content_info(passed->url,(gchar*)query->proxy,(gchar*)query->useragent);
		g_free(passed);
		passed = NULL;
	}
	return head;
}

/*--------------------------------------------------------*/

static void check_all_types_in_url_list(GList * cache_list, GlyrQuery * s)
{
	if(cache_list != NULL)
	{
		GHashTable * thread_id_table = g_hash_table_new(g_direct_hash,g_direct_equal);
		GList * thread_list = NULL;

		glyr_message(2,s,"#[%02d/%02d] Checking image-types: [",s->itemctr,s->number);

		for(GList * elem = cache_list; elem; elem = elem->next)
		{
			GlyrMemCache * item = elem->data;
			if(item != NULL)
			{
				struct wrap_retrieve_pass_data * passer = g_malloc0(sizeof(struct wrap_retrieve_pass_data));
				passer->url   = item->data;
				passer->query = s;

				GThread * thread = g_thread_create((GThreadFunc)wrap_retrieve_content,passer,true,NULL);
				if(thread != NULL)
				{
					thread_list = g_list_prepend(thread_list,thread);
				}
				g_hash_table_insert(thread_id_table,thread,item);
			}
		}

		for(GList * thread = thread_list; thread; thread = thread->next)
		{
			gboolean success = FALSE;
			struct header_data * info = g_thread_join(thread->data);
			if(info != NULL)
			{
				GlyrMemCache * linked_cache = g_hash_table_lookup(thread_id_table,thread->data);
				if(linked_cache != NULL)
				{
					if(g_strcmp0(info->type,"image") == 0)
					{
						linked_cache->img_format = g_strdup(info->format);
						success = TRUE;
					}
				}
				else
				{
					panic("glyr: Uh oh.. empty link in hashtable..\n");
				}
				g_free(info->format);
				g_free(info->type);
				g_free(info->extra);
				g_free(info);
			}

			glyr_message(2,s,"%c",(success) ? '.' : '!');
		}
		g_list_free(thread_list);
		g_hash_table_destroy(thread_id_table);
		glyr_message(2,s,"]");
	}
}

/*--------------------------------------------------------*/

static gboolean format_is_allowed(gchar * format, gchar * allowed)
{
	/* Let everything pass */
	if(allowed == NULL)
	{
		return TRUE;
	}

	gboolean result = FALSE;
	if(format != NULL && allowed != NULL)
	{
		gchar * token;
		gsize offset = 0;
		gsize len = strlen(allowed);

		while(!result && (token = get_next_word(allowed,DEFAULT_FROM_ARGUMENT_DELIM,&offset,len)) != NULL)
		{
			result = (g_strcmp0(token,format) == 0);
			g_free(token);
		}
	}
	return result;
}


/*--------------------------------------------------------*/

static GList * kick_out_wrong_formats(GList * data_list, GlyrQuery * s)
{
	GList * new_head = data_list;	

	/* Parallely check if the format is what we wanted */
	check_all_types_in_url_list(new_head,s);

	gchar * allowed_formats = s->allowed_formats;
	if(allowed_formats == NULL)
	{
		allowed_formats = DEFAULT_ALLOWED_FORMATS;
	}

	/* Now compare it agains the format. */
	gsize invalid_format_counter = 0;
	GList * elem = new_head;
	while(elem != NULL)
	{
		GlyrMemCache * item = elem->data;
		if(item != NULL)
		{
			if(format_is_allowed(item->img_format,allowed_formats) == FALSE)
			{
				GList * to_delete = elem;
				elem = elem->next;
				invalid_format_counter++;

				new_head = g_list_delete_link(new_head,to_delete);
				DL_free(item);
				item = NULL;
				continue;
			}
		}
		elem = elem->next;
	}

	glyr_message(2,s," (-%d item(s) less)\n",invalid_format_counter);
	return new_head;
}

/*--------------------------------------------------------*/

static void do_charset_conversion(MetaDataSource * source, GList * text_list)
{
	if(source != NULL && text_list != NULL)
	{
		for(GList * elem = text_list; elem; elem = elem->next)
		{
			GlyrMemCache * cache = elem->data;

			/* We might need to unescape the HTML Utf8 encoded strings first, this is done later anyway. */
			gchar * utf8_string = unescape_html_UTF8(cache->data);
			if(utf8_string != NULL)
			{
				gsize new_len;
				gchar * conv = convert_charset(utf8_string,"UTF-8",source->encoding,&new_len);
				if(conv != NULL)
				{
					cache->size = new_len;
					g_free(cache->data);
					cache->data = conv;
				}
				g_free(utf8_string);
			}
		}
	}
}

/*--------------------------------------------------------*/

static GList * check_for_forced_utf8(GlyrQuery * query, GList * text_list)
{
	gint deleted = 0;
	GList * new_head = text_list;
	if(query != NULL && text_list != NULL && query->force_utf8 == TRUE)
	{
		glyr_message(2,query,"#[%02d/%02d] Checking encoding [",g_list_length(text_list),query->number);

		GList * elem = text_list;
		while(elem != NULL)
		{
			GlyrMemCache * cache = elem->data;
			const gchar * end_of_valid_utf8 = NULL;
			if(g_utf8_validate(cache->data,-1,&end_of_valid_utf8) == FALSE)
			{
				/* UTF8 was forced, and this cache didn't pass -> deletre */	
				glyr_message(2,query,"!");


				DL_free(cache);
				deleted++;

				GList * to_delete = elem;
				elem = elem->next;
				new_head = g_list_delete_link(new_head,to_delete);
				continue;
			}
			else 
			{
				glyr_message(2,query,".");
				elem = elem->next;
			}
		}
		glyr_message(2,query,"] (-%d item(s) less)\n",deleted);
	}
	return new_head;
} 

/*--------------------------------------------------------*/

static void normalize_utf8(GList * text_list)
{
	for(GList * elem = text_list; elem; elem = elem->next)
	{
		GlyrMemCache * cache = elem->data;
		if(cache != NULL && cache->data)
		{
			if(g_utf8_validate(cache->data,-1,NULL) != FALSE)
			{
				gchar * normalized_utf8 = g_utf8_normalize(cache->data,-1,G_NORMALIZE_NFKC);
				if(normalized_utf8 != NULL)
				{
					/* Swap cache contents */
					g_free(cache->data);
					cache->data = normalized_utf8;
					cache->size = strlen(normalized_utf8);
				}
			}
		}
	}
}

/*--------------------------------------------------------*/

/* The actual call to the metadata provider here, coming from the downloader, triggered by start_engine() */
static GList * call_provider_callback(cb_object * capo, void * userptr, bool * stop_download, gint * to_add)
{
	GList * parsed = NULL;
	if(userptr != NULL)
	{
		/* Get MetaDataSource correlated to this URL */
		GHashTable * assoc = (GHashTable*)userptr;
		MetaDataSource * plugin = g_hash_table_lookup(assoc,capo->url);

		/* Cross fingers... */
		if(plugin != NULL)
		{
			GList * raw_parsed_data = plugin->parser(capo);

			/* Also do some duplicate check already */
			gsize less = delete_dupes(raw_parsed_data,capo->s);
			if(less > 0)
			{
				gsize items_now = g_list_length(raw_parsed_data) + capo->s->itemctr - less;
				glyr_message(2,capo->s,"#[%02d/%02d] Inner check found %ld dupes\n",items_now,capo->s->number,less);
			}

			if(g_list_length(raw_parsed_data) != 0)
			{

				/* We shouldn't check (e.g) lyrics if they are a valid URL ;-) */
				if(capo->s->imagejob == TRUE)
				{
					raw_parsed_data = kick_out_wrong_formats(raw_parsed_data,capo->s);
				}
				else /* We should look if charset conversion is requested */
				{
					normalize_utf8(raw_parsed_data);
					if(plugin->encoding != NULL)
					{
						glyr_message(2,capo->s,"#[%02d/%02d] Attempting to convert charsets\n",g_list_length(raw_parsed_data),capo->s->number);
						do_charset_conversion(plugin, raw_parsed_data);
					}
					raw_parsed_data = check_for_forced_utf8(capo->s,raw_parsed_data);	
				}

				if(g_list_length(raw_parsed_data) != 0)
				{
					for(GList * elem = raw_parsed_data; elem; elem = elem->next)
					{
						GlyrMemCache * item = elem->data;
						if(item != NULL)
						{
							if(capo->s->itemctr < capo->s->number)
							{
								/* Only reference to the plugin -> copy providername */
								item->prov = g_strdup(plugin->name);
								parsed = g_list_prepend(parsed,item);
								capo->s->itemctr++;

							}
							else /* Not needed anymore. Forget this item */
							{
								DL_free(item);
								item = NULL;

								/* Also skip other downloads */
								*stop_download = TRUE;
							}
						}
					}

					/* Forget those pointers */
					g_list_free(raw_parsed_data);
				}
			}
		}
		else
		{
			panic("glyr: hashmap lookup failed. Cannot call plugin => Bug.\n");
		}

	}
	/* We replace the cache with a new one -> free the old one */
	if(capo->cache != NULL)
	{
		DL_free(capo->cache);
		capo->cache = NULL;
	}

	/* Do not add raw data */
	if(parsed == NULL)
	{
		*to_add = 0;
	}

	return parsed;
}

/*--------------------------------------------------------*/

static gboolean provider_is_enabled(GlyrQuery * s, MetaDataSource * f)
{
	/* Assume 'all we have' */
	if(s->from == NULL) {
		return TRUE;
	}

	/* You need to take a little break to read this through at once */
	gboolean is_found    = FALSE;
	gboolean is_excluded = FALSE;
	gboolean all_occured = FALSE;

	/* split string */
	if(f->name != NULL)
	{
		gsize len = strlen(s->from);
		gsize offset = 0;

		gchar * token = NULL;
		while((token = get_next_word(s->from,DEFAULT_FROM_ARGUMENT_DELIM,&offset,len)))
		{
			if(token != NULL)
			{
				gchar * back = token;

				gboolean minus; 
				if((minus = token[0] == '-') || token[0] == '+')
					token++;

				if(!strcasecmp(token,"all"))
					all_occured = TRUE;

				if((token[0] == f->key && strlen(token) == 1) || !strcasecmp(token,f->name))
				{
					is_excluded =  minus;
					is_found    = !minus;
				}
				g_free(back);
			}
		}
	}
	return (all_occured) ? (is_excluded == FALSE) : is_found;
}

/*--------------------------------------------------------*/

/* GnuPlot: plot3d(1/X*Y - (100-Y)*1/(1-X) + 1000,[X,0.1,0.9],[Y,0,100]); */
static gfloat calc_rating(gfloat qsratio, gint quality, gint speed)
{
	gfloat cratio = CLAMP(qsratio,0.1,0.9);
	return 1000.0f + ((1/(1-cratio)*quality) - (1/cratio*speed));
}

/*--------------------------------------------------------*/

static GList * get_queued(GlyrQuery * s, MetaDataFetcher * fetcher, gint * fired)
{
	GList * source_list = NULL;
	for(gint it = 0; it < s->parallel; it++)
	{
		gint pos = 0;
		gint max_pos = -1;
		gfloat max = G_MINFLOAT;

		for(GList * elem = fetcher->provider; elem; elem = elem->next, ++pos)
		{
			MetaDataSource * src = elem->data;
			if(provider_is_enabled(s,src) == TRUE)
			{
				if(fired[pos] == 0)
				{
					gfloat rating = calc_rating(s->qsratio,src->quality,src->speed);
					if(rating > max)
					{
						max = rating;
						max_pos = pos;
					}
				}
			}
		}

		if(max_pos != -1)
		{
			GList * wanted = g_list_nth(fetcher->provider,max_pos);
			if(wanted != NULL)
			{
				MetaDataSource * src = wanted->data;	
				source_list = g_list_prepend(source_list,src);
			}
			fired[max_pos]++;
		}
	}

	if(source_list != NULL)
	{
		source_list = g_list_reverse(source_list);
	}
	return source_list;
}

/*--------------------------------------------------------*/

static GList * prepare_run(GlyrQuery * query, MetaDataFetcher * fetcher, GList * source_list, gboolean * stop_me)
{
	GList * url_list = NULL;
	GList * endmarks = NULL;
	GHashTable * url_table = g_hash_table_new(g_str_hash,g_str_equal);

	/* Iterate over all sources for this fetcher  */
	for(GList * source = source_list; source != NULL; source = source->next)
	{
		MetaDataSource * item = source->data;
		if(item != NULL /*&& provider_is_enabled(query,item)*/)
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
				endmarks = g_list_prepend(endmarks,(gpointer)item->endmarker);

				/* If the URL was dyn. allocated, we should go and free it */
				if(item->free_url == TRUE)
				{
					g_free((gchar*)lookup_url);
				}
			}
		}
	}

	GList * ready_caches = NULL;
	if(g_list_length(url_list) != 0)
	{
		/* Now start the downloadmanager - and call the specified callback with the URL table when an item is ready */
		GList * raw_parsed = async_download(url_list,
				endmarks,
				query,
				g_list_length(url_list)/2,
				1,                         
				call_provider_callback,    /* Callback    */
				url_table,                 /* Userpointer */
				TRUE);

		if(g_list_length(raw_parsed) != 0)
		{

			/* Kill duplicates before finalizing */
			int pre_less = delete_dupes(raw_parsed,query);
			if(pre_less > 0)
			{
				glyr_message(2,query,"- Prefiltering double data: (-%d item(s) less)\n",pre_less);
			}

			glyr_message(2,query,"---- \n");
			if(g_list_length(raw_parsed) != 0)
			{
				/* Call finalize to sanitize data, or download given URLs */
				ready_caches = fetcher->finalize(query, raw_parsed,stop_me);

				/* Raw data not needed anymore */
				g_list_free(raw_parsed);
				raw_parsed = NULL;
			}
		}
	}

	glist_free_full(url_list,g_free);
	url_list = NULL;

	g_list_free(endmarks);
	endmarks = NULL;

	g_hash_table_destroy(url_table);
	return ready_caches;
}

/*--------------------------------------------------------*/

static void print_trigger(GlyrQuery * query, GList * src_list)
{
	glyr_message(2,query,"---- Triggering: ");
	for(GList * elem = src_list; elem; elem = elem->next)
	{
		MetaDataSource * info = elem->data;
		glyr_message(2,query,"%s ",info->name);
	}
	glyr_message(2,query,"\n");
}

/*--------------------------------------------------------*/

GList * start_engine(GlyrQuery * query, MetaDataFetcher * fetcher, enum GLYR_ERROR * err)
{
	gsize list_len = g_list_length(fetcher->provider);
	gint fired[list_len];
	memset(fired,0,list_len * sizeof(gint));

	gboolean something_was_searched = FALSE;
	gboolean stop_now = FALSE;

	GList * src_list = NULL, * result_list = NULL;
	while(
			(stop_now == FALSE)                                   &&
			(g_list_length(result_list) < (gsize)query->number)   &&
			(src_list = get_queued(query, fetcher, fired)) != NULL)
	{
		/* Print what provider were triggered */
		print_trigger(query,src_list);

		/* Sen this list of sources to the download manager */
		GList * sub_list = prepare_run(query,fetcher,src_list, &stop_now);
		if(sub_list != NULL)
		{
			result_list = g_list_concat(result_list,sub_list);
			gint post_less = delete_dupes(result_list,query);

			if(post_less > 0)
			{
				glyr_message(2,query,"- Postfiltering double data: (-%d item(s) less)\n",post_less);
			}
		}

		something_was_searched = TRUE;
		g_list_free(src_list);
		src_list = NULL;
	}

	if(something_was_searched == FALSE)
	{
		if(err != NULL)
		{
			*err = GLYRE_NO_PROVIDER;
		}
	}
	return result_list;
}


/*--------------------------------------------------------*/

void update_md5sum(GlyrMemCache * c)
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

/* As of writing glib 2.24 is pretty common, g_list_free_full() is missing there */
void glist_free_full(GList * List, void (* free_func)(void * ptr))
{
	for(GList * elem = List; elem; elem = elem->next)
	{
		if(free_func != NULL)
		{
			free_func(elem->data);
		}

	}
	g_list_free(List);
}

/*--------------------------------------------------------*/
