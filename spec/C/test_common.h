#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <glib.h>
#include "../../lib/glyr.h"

void setup(GlyrQuery * q, GLYR_GET_TYPE type, int num);
GlyrQuery * setup_alloc(GLYR_GET_TYPE type, int num);
void unsetup(GlyrQuery * q, GlyrMemCache * list);
void init(void);
