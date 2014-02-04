/***********************************************************
 * This file is part of glyr
 * + a commnadline tool and library to download various sort of music related metadata.
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

#include <stdio.h>
#include <glib.h>

#include "../../lib/config.h"
#include "colorprint.h"

/////////////////////////////

static bool global_enable_color = true;

/////////////////////////////

static void printcol (FILE * stream, termcolor fg)
{
    fprintf (stream,"%c[0;%dm", 0x1B, fg + 30);
}

/////////////////////////////

static void resetcolor (FILE * stream)
{
    fprintf (stream,"%c[0m",0x1B);
}

void cprint (termcolor col,gint verbosity, GlyrQuery * s, const char * fmt, ...)
{
    if ( (s && verbosity <= s->verbosity) || verbosity == -1)
    {
        if (col != DEFAULT && global_enable_color)
            printcol (GLYR_OUTPUT,col);

        va_list param;
        va_start (param,fmt);
        g_logv ("Glyrc",G_LOG_LEVEL_INFO,fmt,param);
        va_end (param);

        if (col != DEFAULT && global_enable_color)
            resetcolor (GLYR_OUTPUT);

        fflush(stdout);
    }
}


/////////////////////////////

void enable_color (bool enable)
{
    global_enable_color = enable;
}

/////////////////////////////

void enable_color (bool enable);
