/***********************************************************
 * This file is part of glyr
 * + a commnadline tool and library to download various sort of musicrelated metadata.
 * + Copyright (C) [2011]  [Christopher Pahl]
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

#include <string.h>
#include <stdio.h>
#include <glib.h>

/* All you need from libglyr normally */
#include "../../lib/glyr.h"

/* Update information URL */
#define GLYR_VERSION_URL "http://sahib.github.com/glyr/version.html"

/*----------------------------*/

static gboolean check_version(gboolean * errord)
{
    gboolean result = FALSE;
    GlyrQuery q;
    glyr_query_init(&q);
    glyr_opt_verbosity(&q,3);

    GlyrMemCache * version_page = glyr_download(GLYR_VERSION_URL,&q);
    if(version_page != NULL && version_page->data != NULL)
    {
        glong version = strtol(version_page->data,NULL,10);
        glong current = GLYR_VERSION_MAJOR_INT * 100 + GLYR_VERSION_MINOR_INT * 10 + GLYR_VERSION_MICRO_INT;
        result = (version > current);
    }
    else
    {
        *errord = TRUE;
    }

    glyr_query_destroy(&q);
    return result;
}

/********************************************************************/ 
#define UPDATE_SCRIPT                                                 \
"UPDATE_DIR='/tmp/glyr_update'                                       \n"\
"# Make sure only root can run our script                            \n"\
"if [[ $EUID -ne 0 ]]; then                                          \n"\
"   echo 'This script must be run as root' 1>&2                      \n"\
"   exit 1                                                           \n"\
"fi                                                                  \n"\
"echo '-- Be sure git, cmake and all libraries needed are installed.'\n"\
"echo '-- These are:'                                                \n"\
"echo '--   glib-2.0'                                                \n"\
"echo '--   sqlite3'                                                 \n"\
"echo '--   libcurl'                                                 \n"\
"# Create working dir                                                \n"\
"rm -rf $UPDATE_DIR                                                  \n"\
"mkdir  $UPDATE_DIR                                                  \n"\
"cd     $UPDATE_DIR                                                  \n"\
"# Clone tree                                                        \n"\
"git clone git://github.com/sahib/glyr.git                           \n"\
"# cd && make                                                        \n"\
"cd ${UPDATE_DIR}/glyr                                               \n"\
"cmake . -DCMAKE_INSTALL_PREFIX=/usr                                 \n"\
"make -j 2 install                                                   \n"\
"rm -rf $UPDATE_DIR                                                  \n"\
"echo '=> done'                                                      \n"
/********************************************************************/ 


void update(void)
{
    gboolean error = FALSE;
    if(check_version(&error))
    {
        g_printerr("Updating:\n");
        if(system(UPDATE_SCRIPT) == -1)
        {
            g_printerr("Warning: the script returned a bad exitstatus.\n");
        }
    }
    else if(error)
    {
        g_printerr("Error while updating.\n");
    }
    else
    {
        g_printerr("No updates found.\n");
    }
}

/*-------------------------*/
