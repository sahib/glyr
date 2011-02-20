#include "ruby.h"

#include "../src/glyr.h"

static VALUE rb_mGlyr;
static VALUE rb_cGlyrWriter;

static void Glubyr_mark (genxWriter w)
{}

static void Glubyr_free (genxWriter w)
{
  genxDispose (w);
}

static VALUE writer_allocate (VALUE klass)
{
  genxWriter writer = genxNew (NULL, NULL, NULL);

  return Data_Wrap_Struct (klass, writer_mark, writer_free, writer);
}

void Init_genx4r ()
{
  rb_mGenX = rb_define_module ("GenX");

  rb_cGenXWriter = rb_define_class_under (rb_mGenX, "Writer", rb_cObject);

  /* NOTE: this only works in ruby 1.8.x.  for ruby 1.6.x you instead define
   *       a 'new' method, which does much the same thing as this. */
  rb_define_alloc_func (rb_cGenXWriter, writer_allocate);
}
