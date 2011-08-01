#ifndef GLYR_GENERIC_H
#define GLYR_GENERIC_H

#include <glib.h>
#include "../core.h"
GList * generic_txt_finalizer(GlyQuery * s, GList * list, gboolean * stop_me, enum GLYR_DATA_TYPE type);
GList * generic_img_finalizer(GlyQuery * s, GList * list, gboolean * stop_me, enum GLYR_DATA_TYPE type);

#endif
