#include "../../lib/glyr.h"
#include <glib.h>

/* Simple example to show the use of the glyr_info_get() call */

int main(int argc, const char *argv[])
{
    /* We NEED to init first*/
    glyr_init();
    atexit(glyr_cleanup);

    GlyrFetcherInfo * info = glyr_info_get();
    if(info != NULL)
    {
        /* Iterate over all getters */
        for(GlyrFetcherInfo * elem0 = info; elem0; elem0 = elem0->next)
        {
            /* Iterate over all providers */
            g_print(" %s => %d\n",elem0->name,elem0->type);
            for(GlyrSourceInfo * elem1 = elem0->head; elem1; elem1 = elem1->next)
            {
                g_print("   # %s [%c]\n",elem1->name,elem1->key);
                g_print("     - Quality: %d\n",elem1->quality);
                g_print("     - Speed:   %d\n",elem1->speed);
                g_print("     - Type:    %d\n",elem1->type);
            }

            /* Test which fields are required for a certain getter */
            g_print(" + Requires: (%s%s%s)\n",
		    elem0->reqs & GLYR_REQUIRES_ARTIST ? "Artist " : "",
		    elem0->reqs & GLYR_REQUIRES_ALBUM  ? "Album "  : "",
		    elem0->reqs & GLYR_REQUIRES_TITLE  ? "Title"   : ""
		   );

            /* And which are optional? */
            g_print(" + Optional: (%s%s%s)\n",
		    elem0->reqs & GLYR_OPTIONAL_ARTIST ? "Artist " : "",
		    elem0->reqs & GLYR_OPTIONAL_ALBUM  ? "Album "  : "",
		    elem0->reqs & GLYR_OPTIONAL_TITLE  ? "Title"   : ""
		   );

            g_print("\n///////////////////////////////\n");
        }
    }
    glyr_info_free(info);
    return EXIT_SUCCESS;
}
