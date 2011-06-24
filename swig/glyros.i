/*
 * This is the config file for SWIG, the binding generator for a plethora of languages
 * Language bindings have some drawbacks:
 * - GlyPlug_get_name_by_id is not yet supported, but will come
 * - The callback option does not work. This would make this language specific,
 *   I already tried with ruby, but my C->Ruby is probably not good enough, Patches welcome.
 * Plus points:
 * + By using SWIG you just can use the compile.sh by replacing 'ruby' with <your_lang>
 * + Ruby bindings e.g. are so incredible easy to use after writing a wrapper class 
*/

/* module name */
%module glyros

#define COMING_FROM_SWIG

/* let wrapper file compile */
%{ #include "../lib/glyr.h" %}
  
/* parse headers */
%include "../lib/glyr.h"
%include "../lib/types.h"

/* make sure libglyr wakes  up */
%init 
%{
  Gly_init();
  atexit(Gly_cleanup);
%}
