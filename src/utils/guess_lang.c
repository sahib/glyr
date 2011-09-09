#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <string.h>

const gchar * disallowed_languages[] = {"C","en"};

/**
* @brief Guesses the users language (in ISO639-1 codes like 'de') by the system locale
*
* @return a newly allocated language code. Free.
*/
gchar * guess_lang(void)
{
	/* Default to 'en' in any case */
	gchar * result_lang = g_strdup("en");
#if GLIB_CHECK_VERSION(2,28,0)
	gboolean break_out = FALSE;

	/* Please never ever free this */
	const gchar * const * languages = g_get_language_names();

	for (gint i = 0; languages[i] && !break_out; i++) 
	{
		gchar ** variants = g_get_locale_variants(languages[i]);
		for(gint j = 0; variants[j] && !break_out; j++)
		{
			gboolean allowed_lang = TRUE;
			gint lang_size = (sizeof(disallowed_languages)/sizeof(gchar*));
			for (gint dis = 0; dis < lang_size; dis++)
			{
				if(g_ascii_strncasecmp(disallowed_languages[dis],variants[j],strlen(variants[j])) == 0)
				{
					allowed_lang = FALSE;
				}
			}

			if(allowed_lang && !strchr(variants[j],'@') && !strchr(variants[j],'_'))
			{
				g_free(result_lang);
				result_lang = g_strdup(variants[j]);
				break_out = TRUE;
			}
		}
		g_strfreev(variants);
	}
#else
	puts("GLib version 2.28 is needed to execute this properly.");
#endif
	return result_lang;
}

int main(int argc, const char *argv[])
{
	gchar * lang = guess_lang();
	if(lang != NULL)
	{
		puts(lang);
		g_free(lang);
	}
	return 0;
}
