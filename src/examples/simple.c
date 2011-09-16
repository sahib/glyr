/* Simples possible example for libglyr, see src/example.c for a little more advanced one */
#include <stdlib.h>
#include <stdio.h>

/* you have only to include this. */
#include "../../lib/glyr.h"

/* use this include path for your project, or directly clone libglyr to your project */
//#include <glyr/glyr.h>


int main(void)
{
    /* Init this thing, the only two methods not being threadsafe */
    glyr_init();
    /* Also clear ressources on exit */
    atexit(glyr_cleanup);

    /* This struct is used to store all settings you do via glyr_opt_* */
    GlyrQuery q;
    /* We also should set it to standard values */
    glyr_query_init(&q);

    /* We want lyrics, well, I want. */
    glyr_opt_type(&q,GLYR_GET_LYRICS);

    /* Set random artist/title -  you could also omit the album line */
    glyr_opt_artist(&q,(char*)"Die Ärzte");
    glyr_opt_album (&q,(char*)"Die Bestie in Menschengestalt");
    glyr_opt_title (&q,(char*)"FaFaFa");

    /* If any error occured it will be saved here, or GLYRE_OK */
    /* You could have also passed a NULL pointer to glyr_get() if you're not interested in this */
    GLYR_ERROR err;

    /* Now get the job done! The 3rd  */
    GlyrMemCache * head = glyr_get(&q,&err,NULL);

    /* The results are stored in the GlyrMemCache struct -
       you are most likely only interested in the fields data, size and type*/
    if(head != NULL)
    {
        /* head contains also a pointer to the next element, you can use it therefore as linkedlist */
        puts(head->data);

        /* We have to free it again also, you can pass ANY pointer of the list, it works in both directions */
        glyr_free_list(head);
    }

    /* glyr_query_init  may allocate memory - free it. */
    glyr_query_destroy(&q);
    return err;
}
