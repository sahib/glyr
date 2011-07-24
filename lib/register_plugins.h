#ifndef REGISTER_PLUGINS
  #define REGISTER_PLUGINS
  #include <glib.h>
  void   register_fetcher_plugins(void);
  void unregister_fetcher_plugins(void);

  GList * r_getFList(void);
  GList * r_getSList(void);
#endif
