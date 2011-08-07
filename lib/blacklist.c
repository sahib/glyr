#include "blacklist.h"

/*-----------------------------------------------------------------*/

GHashTable * lookup_table = NULL;
gchar * blacklist_array[] =
{
    "http://ecx.images-amazon.com/images/I/11J2DMYABHL.jpg",       /* blank image */
    "http://cdn.recordshopx.com/cover/normal/5/53/53138.jpg%3Fcd"  /* blank image */
};


/*-----------------------------------------------------------------*/

void blacklist_build(void)
{
    lookup_table = g_hash_table_new(g_str_hash,g_str_equal);
    gint b_size = sizeof(blacklist_array) / sizeof(gchar *);
    for(gint it = 0; it < b_size; it++)
    {
        if(blacklist_array[it] != NULL)
        {
            g_hash_table_insert(lookup_table,blacklist_array[it],blacklist_array[it]);
        }
    }
}

/*-----------------------------------------------------------------*/

void blacklist_destroy(void)
{
    g_hash_table_destroy(lookup_table);
}

/*-----------------------------------------------------------------*/

gboolean is_blacklisted(gchar * URL)
{
    if(lookup_table == NULL || URL == NULL)
        return FALSE;

    return !(g_hash_table_lookup(lookup_table,URL) == NULL);
}

/*-----------------------------------------------------------------*/
