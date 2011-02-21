#include <stdlib.h>
#include <string.h>

#include "kit.h"

#include "../core.h"
#include "../core.h"
#include "../stringop.h"

#define RESULT "<td class=\"kvk-result-txt\">"
#define RESULT_LINK_START "href=\""
#define RESULT_LINK_END "\">"
#define RESULT_START ">"
#define RESULT_END "</a>"

const char * books_kit_url(glyr_settings_t * settings)
{
    return "http://kvk.ubka.uni-karlsruhe.de/hylib-bin/kvk/nph-kvk2.cgi?"
           "maske=kvk-last&"
           "lang=de&"
           "css=none&"
           "input-charset=utf-8&"
           "ALL=%artist%&"
           "Timeout=120&"
           "kataloge=SWB&"
           "kataloge=BVB&"
           "kataloge=NRW&"
           "kataloge=HEBIS&"
           "kataloge=HEBIS_RETRO&"
           "kataloge=KOBV_SOLR&"
           "kataloge=GBV&"
           "kataloge=DDB&"
           "kataloge=STABI_BERLIN&"
           "kataloge=TIB&"
           "kataloge=OEVK_GBV&"
           "kataloge=VD16&"
           "kataloge=VD17&"
           "kataloge=ZDB";
}

static memCache_t * get_field(char ** pointer, char * start, char * end, cb_object * capo)
{
    char * field = NULL;
    field = (field=getStr(pointer, start, end)) == NULL ? "" : field;

    if(field)
    {
        memCache_t * dl = DL_init();
        dl->data = strdup(field);
        dl->size = strlen(dl->data);
        dl->dsrc = strdup(capo->url);
        return dl;
    }
    return NULL;
}


cache_list * books_kit_parse(cb_object * capo)
{
    char * pointer = capo->cache->data;
    cache_list * ls = NULL;

    while ((pointer = strstr(pointer, RESULT)) != NULL)
    {
        pointer+=strlen(RESULT);

        if(!ls) ls = DL_new_lst();

        DL_add_to_list(ls, get_field(&pointer, RESULT_LINK_START, RESULT_LINK_END,capo));
        DL_add_to_list(ls, get_field(&pointer, RESULT_START, RESULT_END,capo));
    }

    return ls;
}
