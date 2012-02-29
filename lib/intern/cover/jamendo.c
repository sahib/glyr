/***********************************************************
 * This file is part of glyr
 * + a commnadline tool and library to download various sort of musicrelated metadata.
 * + Copyright (C) [2011]  [Christopher Pahl]
 * + Hosted at: https://github.com/sahib/glyr
 *
 * Jamendo provider written by:
 *     Christoph Piechula (christoph@nullcat.de)
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

#include "../../stringlib.h"
#include "../../core.h"

#define RESULT_URL "http://www.jamendo.com/get/album/id/album/artworkurl/redirect/%s/?artwork_size=%d"
#define SOURCE_URL "http://api.jamendo.com/get2/id+name+artist_name/album/plain/?order=searchweight_desc&n=100&searchquery=${album}"

static int get_cover_size(GlyrQuery * query);
static bool check_values(GlyrQuery * query, char * artist, char * album);

/* ----------------------------------------------- */

static const char * cover_jamendo_url(GlyrQuery * sets)
{
    return SOURCE_URL;
}

/* ----------------------------------------------- */

static GList * cover_jamendo_parse(cb_object *capo)
{

    GList * result_list = NULL;
    gchar * line = capo->cache->data;

    while(continue_search(g_list_length(result_list),capo->s))
    {
        char * line_end;

        if ((line_end = strchr(line,'\n')) != NULL)
        {
            *line_end = 0;
            char **line_split = g_strsplit(line,"\t",3);

            if (line_split != NULL)
            {
                if (check_values(capo->s,line_split[2],line_split[1]))
                {
                    char * url =  g_strdup_printf(RESULT_URL,line_split[0],get_cover_size(capo->s));
                    GlyrMemCache * result = DL_init();
                    result->data = url;
                    result->size = strlen(url);
                    result_list = g_list_prepend(result_list,result);
                }
            }
            line = ++line_end;
        }
        else
        {
            break;
        }
    }
    return result_list;
}

/* ----------------------------------------------- */

static bool check_values(GlyrQuery * query, char * artist, char * album)
{
    if (levenshtein_strnormcmp(query, query->artist, artist) <= query->fuzzyness
            && levenshtein_strnormcmp(query, query->album,album) <= query->fuzzyness)
    {
        return true;
    }
    return false;
}

/* ----------------------------------------------- */

static int get_cover_size(GlyrQuery * query)
{
    int cover_size[] = {50,50,100,200,300,400,600,INT_MAX};
    int array_len = (sizeof(cover_size)/sizeof(int));

    if (query->img_max_size == -1)
    {
        return 400;

    }
    else
    {
        for(int i=1;i<array_len;i++)
        {
            if (query->img_max_size <= cover_size[i])
            {
                return cover_size[i-1];
            }
        }
    }   
    return 400;
}


/* ----------------------------------------------- */

MetaDataSource cover_jamendo_src =
{
    .name      = "jamendo",
    .key       = 'j',
    .parser    = cover_jamendo_parse,
    .get_url   = cover_jamendo_url,
    .type      = GLYR_GET_COVERART,
    .quality   = 90,
    .speed     = 75,
    .endmarker = NULL,
    .free_url  = false
};
