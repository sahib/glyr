#ifndef GLYR_BLACKLIST_H
#define GLYR_BLACKLIST_H

#include <glib.h>
gboolean is_blacklisted(gchar * URL);
void blacklist_build(void);
void blacklist_destroy(void);

#endif
