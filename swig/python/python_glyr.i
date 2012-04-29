/*
 * This is the config file for SWIG, the binding generator for a plethora of languages
 * Only thing not working right now:
 * glyr_opt_dlcallback() - as it requires some C-ruby magic to get it to work.
*/
 
/* module name */
%module glypy 

/*#define COMING_FROM_SWIG*/

/* let wrapper file compile */
%{ #include "../../lib/glyr.h"    %}
%{ #include "../../lib/cache.h"   %}
%{ #include "../../lib/testing.h" %}

/* parse headers */
%include "../../lib/glyr.h"
%include "../../lib/cache.h"
%include "../../lib/types.h"
%include "../../lib/config.h"
%include "../../lib/testing.h"
%include "glypy.h"

/* make sure libglyr wakes  up */
%init 
%{
  /* User init code */  
  glyr_init();
  atexit(glyr_cleanup);
%}

