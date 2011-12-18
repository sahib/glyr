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

#ifndef GLYR_GENERIC_H
#define GLYR_GENERIC_H

#include <glib.h>
#include "../core.h"
GList * generic_txt_finalizer(GlyrQuery * settings, GList * input_list, gboolean * stop_me, GLYR_DATA_TYPE type, GList ** result_list);
GList * generic_img_finalizer(GlyrQuery * s, GList * list, gboolean * stop_me, GLYR_DATA_TYPE type, GList ** result_list);

#endif
