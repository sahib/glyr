#include <stdlib.h>
#include <stdio.h>

#include <glib.h>
#include "../../lib/glyr.h"

/* --------------------------------------------------
 * Example of using the builtin musictree provider.
 * It searches metadata items in a directory structure,
 * in most cases your music-directory. 
 * A typical example would be the famous 'folder.jpg'.
 *
 * Usage is the same as before, but you have to 
 * use glyr_opt_musictree_path() with a valid path
 * to enable it.
 * -------------------------------------------------- */

static void configure(GlyrQuery * s)
{
    glyr_query_init(s);
    glyr_opt_artist(s,"Die Apokalyptischen Reiter");
    glyr_opt_album (s,"Moral & Wahnsinn");
    glyr_opt_title (s,"Die Boten");
    glyr_opt_verbosity(s,2);
    glyr_opt_type(s,GLYR_GET_COVERART);
}

static void printme(GlyrMemCache * start)
{
    GlyrMemCache * i = start;
    while(i != NULL)
    {
        glyr_cache_print(i);
        puts("--------------------");
        i = i->next;
    }
    glyr_free_list(start);
}

/* Dummy - you will have to replace this yourself */
static gchar * get_music_file_path(void)
{
    /* Relative directories are possible - but should be avoided 
     * Instead the file you could also just pass the containing 
     * directory (- file.mp3 i.e.) 
     * */
    return "src/examples/musictree_dir/artist/album/file.mp3";
}

int main(void)
{
    /* Init */
    glyr_init();
    atexit(glyr_cleanup);

    /* Fill some silly values */
    GlyrQuery query;

    /* Regular stuff */
    configure(&query);

    /* Musictree relevant */

    /* Please refer to the documentation here */
    glyr_opt_musictree_path(&query,get_music_file_path());

    /* We only want musictree */
    glyr_opt_from(&query,"musictree");

    /* Try to get 4 covers - you will get just 2 */
    glyr_opt_number(&query,4);

    GlyrMemCache * cache = glyr_get(&query,NULL,NULL);
    printme(cache);

    return EXIT_SUCCESS;
}
