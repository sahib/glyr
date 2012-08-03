/***********************************************************
 * This file is part of glyr
 * + a commnandline tool and library to download various sort of musicrelated metadata.
 * + Copyright (C) [2011-2012]  [Christopher Pahl]
 * + Hosted at: https://github.com/sahib/glyr
 *
 * glyr is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glyr is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glyr. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************/

#ifndef REGISTER_PLUGINS
#define REGISTER_PLUGINS
#include "types.h"
#include <glib.h>
void   register_fetcher_plugins (void);
void unregister_fetcher_plugins (void);

GList * r_getFList (void);
GList * r_getSList (void);

GlyrFetcherInfo * get_plugin_info (void);
void free_plugin_info (GlyrFetcherInfo * infos);
#endif
