#ifndef REVIEW_H
#define REVIEW_H

#include "types.h"

GlyPlugin   *  glyr_get_review_providers(void);
GlyCacheList * get_review(GlyQuery * settings);

#endif
