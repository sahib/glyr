/***********************************************************
 * This file is part of glyr
 * + a command-line tool and library to download various sort of music related metadata.
 * + Copyright (C) [2011-2012]  [Christopher Pahl]
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

#include <curl/curl.h>
#include <string.h>
#include <glib.h>

/* Don't do this. Use <glyr/glyr.h> */
#include "../../lib/config.h"
#include "../../lib/types.h"

/* Simple testprogram to check the content-type header of any website given on the cmd. */

struct header_data {
    gchar *type;
    gchar *format;
    gchar *extra;
};

/* Parse header file. Get Contenttype from it and save it in the header_data struct */
gsize header_cb(void *ptr, gsize size, gsize nmemb, void *userdata)
{
    gsize bytes = size * nmemb;
    if(ptr != NULL && userdata != NULL) {
        /* Transform safely into string */
        gchar nulbuf[bytes + 1];
        memcpy(nulbuf, ptr, bytes);
        nulbuf[bytes] = '\0';

        /* We're only interested in the content type */
        gchar *cttp  = "Content-Type: ";
        gsize ctt_len = strlen(cttp);
        if(ctt_len < bytes && g_ascii_strncasecmp(cttp, nulbuf, ctt_len) == 0) {
            gchar **content_type = g_strsplit_set(nulbuf + ctt_len, " /;", 0);
            if(content_type != NULL) {
                gsize set_at = 0;
                gchar **elem = content_type;
                struct header_data *info = userdata;

                /* Set fields..  */
                while(elem[0] != NULL) {
                    if(elem[0][0] != '\0') {
                        switch(set_at) {
                        case 0:
                            g_free(info->type);
                            info->type   = g_strdup(elem[0]);
                            break;
                        case 1:
                            g_free(info->format);
                            if(g_ascii_strncasecmp(elem[0], "octet-stream", 12) == 0) {
                                info->format = g_strdup("jpeg");
                            } else {
                                info->format = g_strdup(elem[0]);
                            }
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

/* just prevent writing header to stdout */
gsize empty_cb(void *p, gsize s, gsize n, void *u)
{
    return s * n;
};


void chomp_breakline(gchar *string)
{
    if(string != NULL) {
        gsize len = strlen(string);
        while(--len != 0) {
            if(string[len] == '\n') {
                string[len] = '\0';
                break;
            }
        }
    }
}

static struct header_data *url_is_gettable(gchar *url)
{
    struct header_data *info = NULL;
    if(url != NULL) {
        CURL *eh = curl_easy_init();
        CURLcode rc = CURLE_OK;

        info = g_malloc0(sizeof(struct header_data));

        curl_easy_setopt(eh, CURLOPT_TIMEOUT, 3);
        curl_easy_setopt(eh, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(eh, CURLOPT_USERAGENT, GLYR_DEFAULT_USERAGENT"/linkvalidator");
        curl_easy_setopt(eh, CURLOPT_URL, url);
        curl_easy_setopt(eh, CURLOPT_FOLLOWLOCATION, TRUE);
        curl_easy_setopt(eh, CURLOPT_HEADER, TRUE);
        curl_easy_setopt(eh, CURLOPT_NOBODY, TRUE);
        curl_easy_setopt(eh, CURLOPT_HEADERFUNCTION, header_cb);
        curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, empty_cb);
        curl_easy_setopt(eh, CURLOPT_WRITEHEADER, info);

        curl_easy_setopt(eh, CURLOPT_VERBOSE, TRUE);

        rc = curl_easy_perform(eh);
        curl_easy_cleanup(eh);

        if(rc != CURLE_OK) {
            g_printerr("glyr: ping: E: %s [%d]\n", curl_easy_strerror(rc), rc);
            g_free(info);
            info = NULL;

        } else {
            chomp_breakline(info->type);
            chomp_breakline(info->format);
            chomp_breakline(info->extra);
        }
    }
    return info;
}

int main(int argc, char *argv[])
{
    curl_global_init(CURL_GLOBAL_ALL);
    if(argc > 1) {
        struct header_data *info;
        if((info = url_is_gettable(argv[1])) != NULL) {
            g_print("Type  :  %s\n", info->type);
            g_print("Format:  %s\n", info->format);
            g_print("Extra :  %s\n", info->extra);

            g_free(info->type);
            g_free(info->format);
            g_free(info->extra);
            g_free(info);
        } else {
            g_print("Unexists.\n");
        }
    }
    curl_global_cleanup();
    return 1;
}
