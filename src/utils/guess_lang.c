/***********************************************************
 * This file is part of glyr
 * + a command-line tool and library to download various sort of music related metadata.
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

#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <string.h>

const gchar * map_language[][2] =
{
    {"en_US","us"},
    {"en_CA","ca"},
    {"en_UK","uk"}
};

#define END_STRING(STR,CHAR) {gchar * term = strchr(STR,CHAR); if(term) *term = 0;}

/**
* @brief Guesses the users language (in ISO639-1 codes like 'de') by the system locale
*
* @return a newly allocated language code. Free.
*/
gchar * guess_lang (void)
{
    /* Default to 'en' in any case */
    gchar * result_lang = g_strdup ("en");

#if GLIB_CHECK_VERSION(2,28,0)
    gboolean break_out = FALSE;

    /* Please never ever free this */
    const gchar * const * languages = g_get_language_names();

    for (gint i = 0; languages[i] && break_out == FALSE; i++)
    {
        gchar ** variants = g_get_locale_variants (languages[i]);
        for (gint j = 0; variants[j] && !break_out; j++)
        {
            /* Look up if we need to map a language */
            gchar * to_investigate = variants[j];
            gint map_size = (sizeof (map_language) / (2 * sizeof (char*) ) );
            for (gint map = 0; map < map_size; map++)
            {
                const gchar * to_map = map_language[map][0];
                gsize map_len  = strlen (to_map);
                if (g_ascii_strncasecmp (to_map,to_investigate,map_len) == 0)
                {
                    to_investigate = (gchar*) map_language[map][1];
                    g_print ("Mapping language to %s\n",to_investigate);
                    break;
                }
            }

            gboolean allowed_lang = TRUE;

            if (allowed_lang &&
                    g_ascii_strncasecmp ("en",to_investigate,2) != 0 &&
                    g_ascii_strncasecmp ("C", to_investigate,1) != 0 &&
                    !strchr (to_investigate,'@') && !strchr (to_investigate,'.') )
            {
                g_free (result_lang);
                result_lang = g_strdup (to_investigate);
                break_out = TRUE;
            }
        }
        g_strfreev (variants);
    }

#elif GLIB_CHECK_VERSION(2,26,0)

    /* Fallback to simpler version of the above,
         * g_get_locale_variants is not there in this version
         */
    const gchar * const * possible_locales = g_get_language_names();
    if (possible_locales != NULL)
    {
        /* might be a bit weird */
        for (gint i = 0; possible_locales[i]; i++)
        {
            if (g_ascii_strncasecmp ("en",possible_locales[i],2) != 0 &&
                    g_ascii_strncasecmp ("C", possible_locales[i],1) != 0)
            {
                g_free (result_lang);
                result_lang = g_strdup (possible_locales[i]);
                break;
            }
        }
    }
#else
    /* Fallback for version prior GLib 2.26:
     * Fallback to "en" always.
     * Gaaah... shame on you if this happens to you ;-)
     */
    puts ("GLib version 2.28 is needed to execute this properly.");
#endif

    /* Properly terminate string */
    END_STRING (result_lang,'_');
    END_STRING (result_lang,'@');
    END_STRING (result_lang,'.');

    /* We don't need it anymore */
#undef END_STRING

    return result_lang;
}

int main (int argc, const char *argv[])
{
    gchar * lang = guess_lang();
    if (lang != NULL)
    {
        puts (lang);
        g_free (lang);
    }
    return 0;
}
