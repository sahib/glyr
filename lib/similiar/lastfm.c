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

#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "lastfm.h"

#include "../cover.h"
#include "../stringop.h"
#include "../core.h"
#include "../core.h"

#define API_KEY API_KEY_LASTFM

const char * similiar_lastfm_url(GlyQuery * sets)
{
    return "http://ws.audioscrobbler.com/2.0/?method=artist.getsimilar&artist=%artist%&api_key="API_KEY;
}

#define NAME_BEGIN "<name>"
#define NAME_ENDIN "</name>"

#define MATCH_BEGIN "<match>"
#define MATCH_ENDIN "</match>"

#define URL_BEGIN "<url>"
#define URL_ENDIN "</url>"

#define IMAGE_S_BEGIN "<image size=\"small\">"
#define IMAGE_M_BEGIN "<image size=\"medium\">"
#define IMAGE_L_BEGIN "<image size=\"large\">"
#define IMAGE_E_BEGIN "<image size=\"extralarge\">"
#define IMAGE_X_BEGIN "<image size=\"mega\">"
#define IMAGE_ENDIN "</image>"

static char * in_tag(const char * string, const char * begin, const char * endin)
{
    char * bp = strstr(string,begin);
    if(bp != NULL)
    {
        char * ep = strstr(bp,endin);
        if(ep != NULL && ep > bp)
        {
            return copy_value(bp+strlen(begin),ep);
        }
    }
    return NULL;
}

GlyCacheList * similiar_lastfm_parse(cb_object * capo)
{
    GlyCacheList * r_list = NULL;

    int urlc = 0;

    char * find = capo->cache->data;
    while( (find = strstr(find+1, "<artist>")) != NULL && continue_search(urlc,capo->s) && continue_search(urlc,capo->s))
    {
        char * name  = in_tag(find,NAME_BEGIN,NAME_ENDIN);
        char * match = in_tag(find,MATCH_BEGIN,MATCH_ENDIN);
        char * url   = in_tag(find,URL_BEGIN,URL_ENDIN);

        char * img_s = in_tag(find,IMAGE_S_BEGIN,IMAGE_ENDIN);
        char * img_m = in_tag(find,IMAGE_M_BEGIN,IMAGE_ENDIN);
        char * img_l = in_tag(find,IMAGE_L_BEGIN,IMAGE_ENDIN);
        char * img_e = in_tag(find,IMAGE_E_BEGIN,IMAGE_ENDIN);
        char * img_x = in_tag(find,IMAGE_X_BEGIN,IMAGE_ENDIN);

        char * composed = strdup_printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",name,match,url,img_s,img_m,img_l,img_e,img_x);

        if(composed != NULL)
        {
            GlyMemCache * result = DL_init();
            if(!r_list) r_list = DL_new_lst();

            result->data = composed;
            result->size = strlen(composed);
            result->dsrc = strdup(capo->url);

            DL_add_to_list(r_list, result);

            urlc++;
        }

        if(name)
            free(name);
        if(match)
            free(match);
        if(url)
            free(url);
        if(img_s)
            free(img_s);
        if(img_m)
            free(img_m);
        if(img_l)
            free(img_l);
        if(img_e)
            free(img_e);
        if(img_x)
            free(img_x);

    }
    return r_list;
}
