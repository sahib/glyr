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
#include "musicbrainz.h"
#include "../stringlib.h"

#include "../tags/musicbrainz.h"

/*--------------------------------------------------------*/

#define RELATION_BEGIN_TYPE "<relation type=\""

/* Wrap around the (a bit more) generic versions */
GlyCacheList * relations_musicbrainz_parse(cb_object * capo)
{
        GlyCacheList * results = NULL;
        GlyMemCache  * infobuf = generic_musicbrainz_parse(capo,"url-rels");

        if(infobuf) {
                size_t nlen = strlen(RELATION_BEGIN_TYPE);
                char * node = strstr(infobuf->data,"<relation-list target-type=\"Url\">");
                if(node != NULL) {
                        int ctr = 0;
                        while(continue_search(ctr,capo->s) && (node = strstr(node+1,RELATION_BEGIN_TYPE)) ) {
                                char * end_of_type = strchr(node+nlen,'"');
                                if(!end_of_type)
                                        continue;

                                char * beg_of_target = strchr(end_of_type+1,'"');
                                if(!beg_of_target)
                                        continue;

                                char * end_of_target = strchr(beg_of_target+1,'"');
                                if(!end_of_target)
                                        continue;

                                char * type = copy_value(node+nlen,end_of_type);
                                char * target = copy_value(beg_of_target+1,end_of_target);
                                if(type != NULL && target != NULL) {
                                        GlyMemCache * tmp = DL_init();
                                        tmp->data = strdup_printf("%s:%s",type,target);
                                        tmp->size = strlen(tmp->data);
                                        tmp->type = TYPE_RELATION;
                                        tmp->dsrc = infobuf->dsrc ? strdup(infobuf->dsrc) : NULL;

                                        if(!results) results = DL_new_lst();
                                        DL_add_to_list(results,tmp);

                                        ctr++;

                                        free(type);
                                        free(target);
                                }
                        }
                }
                DL_free(infobuf);
        }
        return results;
}

/*--------------------------------------------------------*/

const char * relations_musicbrainz_url(GlyQuery * sets)
{
        return generic_musicbrainz_url(sets);
}

/*--------------------------------------------------------*/
