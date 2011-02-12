#include <stdlib.h>
#include <string.h>

#include "dnb.h"

#include "../types.h"
#include "../core.h"
#include "../stringop.h"

#define TABLE_START "<table valign=\"bottom\" cellpadding=\"3\" cellspacing=\"0\" class=\"yellow\" width=\"100%\" summary=\"Vollanzeige des Suchergebnises\">"
#define TABLE_END "</table>"

#define COLUMN_KEY "<td width=\"25%\""
#define COLUMN_KEY_START ">"
#define COLUMN_KEY_END "</td>"
#define COLUMN_VALUE "<td"
#define COLUMN_VALUE_START ">"
#define COLUMN_VALUE_END "</td>"

#define URL "https://portal.d-nb.de/opac.htm?method=showFullRecord&currentResultId=%artist%%2526any&currentPosition=0"
#define DOMAIN "d-nb."

const char * books_dnb_url(glyr_settings_t * settings)
{
    if (strcmp(settings->artist, "LINK") == 0)
    {
        if (strstr(settings->album, DOMAIN) != NULL)
        {
            return settings->album;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return URL;
    }
}

memCache_t * books_dnb_parse(cb_object * capo)
{
    char * column = NULL;
    char * pointer = capo->cache->data;

    pointer = getStr(&pointer, TABLE_START, TABLE_END);

    if (pointer == NULL)
    {
        return NULL;
    }

    while (1)
    {
        if ((pointer = strstr(pointer, COLUMN_KEY)) == NULL)
        {
            break;
        }

        column = (column=getStr(&pointer, COLUMN_KEY_START, COLUMN_KEY_END)) == NULL ? "" : column;
        column = remove_html_tags_from_string(column, strlen(column));
        trim_inplace(column);
        printf("%s", column);
        free(column);

        if ((pointer = strstr(pointer, COLUMN_VALUE)) == NULL)
        {
            break;
        }

        column = (column=getStr(&pointer, COLUMN_VALUE_START, COLUMN_VALUE_END)) == NULL ? "" : column;
        column = remove_html_tags_from_string(column, strlen(column));
        trim_inplace(column);
        printf(";%s\n", column);
        free(column);
    }

    /*
    memCache_t * r_cache = DL_init();
    r_cache->data = strdup(start);
    r_cache->size = strlen(start);
    return r_cache;
    */
    return NULL;
}
