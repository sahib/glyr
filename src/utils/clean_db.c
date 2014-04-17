#include "../../lib/glyr.h"
#include "../../lib/cache.h"

#include <glib.h>
#include <string.h>

static bool do_delete = false;

static int foreach_callback(GlyrQuery *q, GlyrMemCache *item, void *userptr)
{
    GlyrDatabase *db = (GlyrDatabase *) userptr;

    g_assert(item);
    g_assert(db);

    if(item->rating == -1) {
        g_printerr("----------------\n");
        glyr_cache_print(item);

        if(do_delete) {
            glyr_db_delete(db, q);
        }
    }

    return 0;
}

int main(int argc, char const *argv[])
{

    glyr_init();
    atexit(glyr_cleanup);

    if(argc > 1) {

        GlyrDatabase *db = glyr_db_init(argv[1]);
        if(db != NULL) {

            if(argv[2] != NULL && strcmp(argv[2], "delete") == 0) {
                do_delete = true;
            }

            glyr_db_foreach(db, foreach_callback, db);
        } else {
            g_message("Could not open DB at %s", argv[1]);
        }
    } else {

        g_message("Usage: %s /path/to/db/directory [delete]", argv[0]);
    }
    return EXIT_SUCCESS;
}
