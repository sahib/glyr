/* Simples possible example for libglyr, see src/example.c for a little more advanced one */
#include <stdlib.h>
#include <stdio.h>

/* you have only to include this. */
//#include "../lib/glyr.h"

/* use this include path for your project, or directly clone libglyr to your project */
#include <glyr/glyr.h>


int main(void) {
	/* Init this thing, the only two methods not being threadsafe */
	Gly_init();
	/* Also clear ressources on exit */
	atexit(Gly_cleanup);

	/* This struct is used to store all settings you do via GlyOpt_* */
	GlyQuery q;
	/* We also should set it to standard values */
	Gly_init_query(&q);

	/* We want lyrics, well, I want. */
	GlyOpt_type(&q,GET_LYRICS);

	/* Set random artist/title -  you could also omit the album line */
        GlyOpt_artist(&q,(char*)"Die Ärzte");
        GlyOpt_album (&q,(char*)"Die Bestie in Menschengestalt");
        GlyOpt_title (&q,(char*)"FaFaFa");

	/* If any error occured it will be saved here, or GLYRE_OK */
	/* You could have also passed a NULL pointer to Gly_get() if you're not interested in this */
	enum GLYR_ERROR err;

	/* Now get the job done! The 3rd  */
	GlyMemCache * head = Gly_get(&q,&err,NULL);

	/* The results are stored in the GlyMemCache struct - 
	   you are most likely only interested in the fields data, size and type*/
	if(head != NULL) {
		/* head contains also a pointer to the next element, you can use it therefore as linkedlist */ 
		puts(head->data);

		/* We have to free it again also, you can pass ANY pointer of the list, it works in both directions */
		Gly_free_list(head);
	}

	/* Gly_init_query  may allocate memory - free it. */
	Gly_destroy_query(&q);
	return err;
} 
