/*
 * This is the config file for SWIG, the binding generator for a plethora of languages
 * Only thing not working right now:
 * glyr_opt_dlcallback() - as it requires some C-ruby magic to get it to work.
*/
 
/* module name */
%module glyros

/* Inform headers */
#define COMING_FROM_SWIG

/* let wrapper file compile */
%{ #include "../lib/glyr.h"  %}
%{ #include "../lib/cache.h" %}
%{ #include "../lib/testing.h" %}

/* parse headers */
%include "../lib/glyr.h"
%include "../lib/cache.h"
%include "../lib/types.h"
%include "../lib/config.h"
%include "../lib/testing.h"

%{

  /*---------------------------------*/

  typedef struct _cc
  {
       VALUE query;
       VALUE proc;
  } callee_carrier;

  /*---------------------------------*/

  static GLYR_ERROR callee_wrapper(GlyrMemCache * c_cache, GlyrQuery * c_query)
  {
       /* Get callback data */
       callee_carrier * carr = c_query->callback.user_pointer;
       if(carr != NULL && carr->proc)
       {
           /* Convert c_cache to a ruby memcache */
           VALUE RMemCache = SWIG_NewPointerObj(SWIG_as_voidptr(c_cache), SWIGTYPE_p__GlyrMemCache, 0 |  0 );

           /* Call the proc */
           VALUE result = rb_funcall(carr->proc, rb_intern("call"), 2, RMemCache, carr->query);

           /* Check proc's return */
           if(result != Qnil)
           {
               return FIX2INT(result);
           }
       }
      
       /* Default */
       return GLYRE_OK;
  }

  /*---------------------------------*/

  static VALUE callee_set_callback(VALUE self, VALUE query, VALUE proc)
  {
     /* Convert Ruby-GlyrQuery to C-GlyrQuery */
     void * query_ptr;
     int result = SWIG_ConvertPtr(query, &query_ptr,SWIGTYPE_p__GlyrQuery, 0 |  0 );

     /* Typecheck */
     if (!SWIG_IsOK(result)) 
     {
          SWIG_exception_fail(SWIG_ArgError(result), Ruby_Format_TypeError( "", "GlyrQuery * ","glyr_opt_dlcallback", 1, self )); 
     }
     else
     {
         /* Collect the callback data */
         callee_carrier * carr = malloc(sizeof(callee_carrier));
         carr->query = query;
         carr->proc  = proc;

         /* Free old userpointer. As a user of this library you shouldn't use this. 
          * I'm the autor. I'm allowed to do this ;-)
          */
         if(((GlyrQuery*) query_ptr)->callback.user_pointer != NULL)
         {
            free(((GlyrQuery *)query_ptr)->callback.user_pointer);
         }

         /* Register the callback */
         glyr_opt_dlcallback(query_ptr,callee_wrapper,carr);
     }

     /* return nil */
     fail:
     return Qnil;
  }

  /*---------------------------------*/
  /*---------------------------------*/
  /*---------------------------------*/

  static int db_foreach_proxy_callback(GlyrQuery * c_query, GlyrMemCache * c_cache, void * user_ptr)
  {
        VALUE proc = (VALUE)user_ptr;
        VALUE RMemCache = SWIG_NewPointerObj(SWIG_as_voidptr(c_cache), SWIGTYPE_p__GlyrMemCache, 0 |  0 );
        VALUE RQueryTYP = SWIG_NewPointerObj(SWIG_as_voidptr(c_query), SWIGTYPE_p__GlyrQuery,    0 |  0 );

        /* Call the proc */
        VALUE result = rb_funcall(proc, rb_intern("call"), 2,RQueryTYP, RMemCache);
        if(result != Qnil)
        {
           return FIX2INT(result);
        }
        else return 0;
  }

  /*---------------------------------*/

  static VALUE db_foreach_set_callback(VALUE self, VALUE db, VALUE proc)
  {
     /* Convert Ruby-GlyrDatabase to C-GlyrDatabase */
     void * db_ptr;
     int result = SWIG_ConvertPtr(db, &db_ptr,SWIGTYPE_p__GlyrDatabase, 0 |  0 );
       
     /* Typecheck */
     if (!SWIG_IsOK(result)) 
     {
          SWIG_exception_fail(SWIG_ArgError(result), Ruby_Format_TypeError( "", "GlyrDatabase * ","glyr_db_foreach", 1, self )); 
     }
     else
     {
          glyr_db_foreach(db_ptr, db_foreach_proxy_callback, (void*)proc);
     }

     /* return nil */
     fail:
     return Qnil;
  }
    
%}

/* make sure libglyr wakes  up */
%init 
%{
  glyr_init();
  atexit(glyr_cleanup);

  /* Make callback work */
  rb_undef_method(mGlyros,"glyr_opt_dlcallback");
  rb_define_module_function(mGlyros, "glyr_opt_dlcallback",callee_set_callback,2);

  rb_undef_method(mGlyros,"glyr_db_foreach");
  rb_define_module_function(mGlyros, "glyr_db_foreach",db_foreach_set_callback,2);
%}

