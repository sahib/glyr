#ifndef REGISTER_PLUGINS
#define REGISTER_PLUGINS
#include "types.h"
#include <glib.h>
void   register_fetcher_plugins(void);
void unregister_fetcher_plugins(void);

GList * r_getFList(void);
GList * r_getSList(void);

GlyrFetcherInfo * get_plugin_info(void);
void free_plugin_info(GlyrFetcherInfo * infos);
#endif
