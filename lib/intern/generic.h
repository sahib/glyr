#ifndef GLYR_GENERIC_H
#define GLYR_GENERIC_H

#include <glib.h>
#include "../core.h"
GList * generic_txt_finalizer(GlyrQuery * settings, GList * input_list, gboolean * stop_me, GLYR_DATA_TYPE type, GList ** result_list);
GList * generic_img_finalizer(GlyrQuery * s, GList * list, gboolean * stop_me, GLYR_DATA_TYPE type, GList ** result_list);

#endif
