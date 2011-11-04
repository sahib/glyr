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
#include <ctype.h>

/* For curl_easy_escape() */
#include <curl/curl.h>

#include "stringlib.h"
#include "core.h"
#include "types.h"

/* Implementation of the Levenshtein distance algorithm
 * Compare the number of edits needed to convert string $s
 * to target string $t
 * This is used to correct typos in e.g: lyricswiki
 * If you're a member of lyricswiki:
 * STOP MAKING TYPOS! IT TOOK ME OVER AN HOUR TO WRITE THIS! :-P
 * (not meant to be serious, you guys are cool ;-))
 * For instructions go to http://www.merriampark.com/ld.htm
 * They have also Source code in C++, Java & VB
 */
gsize levenshtein_strcmp(const gchar * s, const gchar * t)
{
    int n = (s) ? strlen(s)+1 : 0;
    int m = (t) ? strlen(t)+1 : 0;

    // Nothing to compute really..
    if (n==0) return m;
    if (m==0) return n;

    // String matrix
    int d[n][m];
    int i,j;

    // Init first row|column to 0...n|m
    for (i=0; i<n; i++) d[i][0] = i;
    for (j=0; j<m; j++) d[0][j] = j;

    for (i=1; i<n; i++)
    {
        // Current char in string s
        char cats = s[i-1];

        for (j=1; j<m; j++)
        {
            // Do -1 only once
            int jm1 = j-1,
                im1 = i-1;

            // a = above cell, b = left cell, c = left above celli
            int a = d[im1][j] + 1,
                b = d[i][jm1] + 1,
                c = d[im1][jm1] + (t[jm1] != cats);

            // Now compute the minimum of a,b,c and set MIN(a,b,c) to cell d[i][j]
 	    d[i][j] = (a < b) ? MIN(a,c) : MIN(b,c);
        }
    }

    // The result is stored in the very right down cell
    return d[n-1][m-1];
}

/* ------------------------------------------------------------- */

/**
 * @brief Strips lint like "feat.", "CD1" etc.
 *
 * @param the string
 *
 * @return a newly allocated string
 */
const gchar * const regex_table[][2] = {
	{"CD[[:blank:]]*[0-9]+",   ""}, /* 'CD 1'  -> ''    */
	{"track[[:blank:]]*[0-9]+",""}, /* 'CD 1'  -> ''    */
	{"(`|'|\"|\\.|,)",         ""}, /* Punctuation.     */
	{"(\\(|)feat(\\.|uring).*",""}, /* "feat." -> " "   */
	{"[[:space:]]{2,}",       " "}  /* 'a  b'  -> 'a b' */
};	

const gsize regex_table_size = sizeof(regex_table) / (2 * sizeof(gchar*));

gchar * regex_replace_by_table(const gchar * string, const gchar * const delete_string[][2], gsize string_size)
{
	gchar * result_string = (gchar*)string;

	/* Match strings to delete */
	for(gsize it = 0; it < string_size; it++)
	{
		GError * match_error = NULL;
		GRegex * regex = g_regex_new(delete_string[it][0],G_REGEX_CASELESS /*| G_REGEX_MULTILINE*/,0,&match_error);

		if(regex != NULL)
		{
			GMatchInfo * match_info;
			g_regex_match_full(regex, string, -1, 0, 0, &match_info,&match_error);
			while (g_match_info_matches (match_info))
			{
			      gchar *word = g_match_info_fetch(match_info, 0);
			      gchar * old_memory = result_string;
			      result_string = strreplace(old_memory,word,delete_string[it][1]);
			      if(old_memory != string)
			      {
				    g_free(old_memory);
			      }

			      g_match_info_next(match_info, &match_error);
			      g_free (word);
			}
			g_match_info_free(match_info);
			g_regex_unref(regex);
		}

		if(match_error != NULL)
    		{
      			glyr_message(-1,NULL,"glyr: Error while matching: %s\n", match_error->message);
      			g_error_free(match_error);
    		}
	}

	/* Trim it the old way - we have to dup the string anyway */
	if(result_string != NULL)
	{
		gsize result_len = strlen(result_string);  
        	gchar * trim = g_malloc0(result_len + 1);
        	trim_copy(result_string,trim);
		if(result_string != string)
		{
			g_free(result_string);
		}
		result_string = trim;
	}

	return result_string;
}

/* ------------------------------------------------------------- */

gsize levenshtein_strcasecmp(const gchar * string, const gchar * other)
{
    gsize diff = 100;
    if(string != NULL && other != NULL)
    {
        /* Lowercase UTF8 string might have more or less bytes! */
        gchar * lower_string = g_utf8_strdown(string,-1);
        gchar * lower_other  = g_utf8_strdown(other, -1);

	if(lower_string && lower_other)
	{
		gchar * normalized_lower = g_utf8_normalize(lower_string,-1,G_NORMALIZE_NFKC);
		gchar * normalized_other = g_utf8_normalize(lower_other, -1,G_NORMALIZE_NFKC);
		if(normalized_lower && normalized_other)
		{
        		diff = levenshtein_strcmp(normalized_lower, normalized_other);
		}
		g_free(normalized_lower);
		g_free(normalized_other);
	}

        /* Free 'em */
        g_free(lower_string);
        g_free(lower_other);
    }
    return diff;
}

/* ------------------------------------------------------------- */

/* Tries to strip unused strings before comparing with levenshtein_strcasecmp */
gsize levenshtein_strnormcmp(GlyrQuery * settings, const gchar * string, const gchar * other)
{
	gsize diff = 100;
	if(string != NULL && other != NULL)
	{
        gchar * unwinded_string = unwind_artist_name(string);
        gchar * unwinded_other  = unwind_artist_name(other);
        if(unwinded_string && unwinded_other)
        {
            gchar * norm_string = regex_replace_by_table(unwinded_string,regex_table,regex_table_size);
            gchar * norm_other  = regex_replace_by_table(unwinded_other, regex_table,regex_table_size);
            if(norm_string && norm_other)
            {
                gchar * pretty_string = beautify_string(norm_string);
                gchar * pretty_other  = beautify_string(norm_other);

                if(pretty_string && pretty_other)
                {
                    remove_tags_from_string(pretty_string,-1,'(',')');
                    remove_tags_from_string(pretty_other,-1, '(',')');

                    diff = levenshtein_strcasecmp(pretty_string,pretty_other);

                    /* Apply correction */
                    gsize str_len = strlen(pretty_string);
                    gsize oth_len = strlen(pretty_other);
                    gsize ratio = (oth_len + str_len) / 2;
                    gsize fuzz  = (settings) ? settings->fuzzyness : GLYR_DEFAULT_FUZZYNESS;

                    /* Useful for debugging */
                    //g_print("%d:%s <=> %d:%s -> %d\n",(gint)str_len,string,(gint)oth_len,other,(gint)diff);

                    if((ratio - diff < ratio / 2 + 1 && diff <= fuzz) || MIN(str_len,oth_len) <= diff)
                    {
                        /* Examples: Adios <=> Weiß or 19 <=> 21 pass levenshtein_strcasecmp */
                        //g_print("warn: The strings might accidentally pass levenshtein: %s <=> %s = %d\n",pretty_string,pretty_other,(gint)diff);
                        diff += 100;
                    }
                }

                g_free(pretty_string);
                g_free(pretty_other);
            }
            g_free(norm_string);
            g_free(norm_other);
        }
        g_free(unwinded_string);
        g_free(unwinded_other);
    }
    return diff;
}

/* ------------------------------------------------------------- */

gchar * strreplace(const char * string, const char * subs, const char * with)
{
    gchar * result = NULL;
    if(string != NULL && string[0] != '\0')
    {
        gchar ** split = g_strsplit(string,subs,0);
        if(split != NULL)
        {
            result = g_strjoinv(with,split);
        }
        g_strfreev(split);
    }
    return result;
}

/* ------------------------------------------------------------- */

/* "Clapton, Eric" -> "Eric Clapton"
 * Cheers Christoph for writing this.
 */
gchar * unwind_artist_name(const gchar * artist)
{
    if(NULL == artist)
    {
        return NULL;
    }

    /*split string in two*/
    gchar * separator = strchr(artist, ',');

    if (separator)
    {
        gchar * result = NULL; 
        gchar ** strsplit = g_strsplit(artist, ",", 2);
        if((strsplit[0] != NULL) && (strsplit[1] != NULL))
        {
            char * tmp = NULL;

            tmp = strsplit[0];
            strsplit[0] = strsplit[1];
            strsplit[1] = tmp;
        }

        result = g_strjoinv(" ",strsplit);

        /*free g_strsplit memory*/
        g_strfreev(strsplit);
        return result;
    }
    else
    {
        /* nothing done, return just copy*/
        return g_strdup(artist);
    }
}

/* ------------------------------------------------------------- */

gchar * prepare_string(const gchar * input, gboolean delintify, gboolean do_curl_escape)
{
    gchar * result = NULL;
    if(input != NULL)
    {
        gchar * downed = g_utf8_strdown(input,-1);
        if(downed != NULL)
        {
            gchar * normalized = g_utf8_normalize(downed,-1,G_NORMALIZE_NFKC);
            if(normalized != NULL)
            {
                gchar * no_lint = regex_replace_by_table(normalized,regex_table,regex_table_size);
                if(no_lint != NULL)
                {
                    if(do_curl_escape)
                    {
                        result = curl_easy_escape(NULL,no_lint,0);
                    }
                    else
                    {
                        result = no_lint;
                    }

                    if(result != NULL && delintify == TRUE)
                    {
                        remove_tags_from_string(result,-1,'(',')');
                    }

                    if(do_curl_escape == TRUE) g_free(no_lint);
                }
                g_free(normalized);
            }
            g_free(downed);
        }
    }
    return result;
}

static void swap_string(char ** tmp, const char * subs, const char * with)
{
    char * swap = *tmp;
    *tmp = strreplace(swap,subs,with);
    g_free(swap);
}

/* Prepares the url for you to get downloaded. You don't have to call this. */
gchar * prepare_url(const gchar * URL, GlyrQuery * s, gboolean do_curl_escape)
{
    gchar * tmp = NULL;
    if(URL != NULL && s != NULL)
    {
        tmp = g_strdup(URL);

        gchar * unwinded_artist = unwind_artist_name(s->artist);

        gchar * p_artist = prepare_string(trim_nocopy(unwinded_artist),FALSE,do_curl_escape);
        gchar * p_album  = prepare_string(s->album,TRUE,do_curl_escape);
        gchar * p_title  = prepare_string(s->title,TRUE,do_curl_escape);

        swap_string(&tmp,"${artist}",p_artist);
        swap_string(&tmp,"${album}", p_album);
        swap_string(&tmp,"${title}", p_title);

        g_free(p_artist);
        g_free(p_album);
        g_free(p_title);
        g_free(unwinded_artist);
    }
    return tmp;
}

/* ------------------------------------------------------------- */

gchar * get_next_word(const gchar * string, const gchar * delim, gsize *offset, gsize len)
{
    gchar * word = NULL;
    if(string && delim && *(offset) < len)
    {
        gchar * occurence = strstr(string+(*offset),delim);
        if(occurence)
        {
            word=copy_value(string+(*offset),occurence);
            *(offset) += (1 + occurence - (string + *(offset)));
        }
        else
        {
            word=copy_value(string+(*offset),string+len);
            *(offset) = len;
        }
    }
    if(word)
    {
        gchar * trim = g_malloc0(len + 1);
        trim_copy(word,trim);
        free(word);
        word = trim;
    }
    return word;
}

/* ------------------------------------------------------------- */

static int convert_to_number(const gchar * string)
{
    if(string != NULL)
    {
        return strtoul(string, NULL, (string[0] == 'x' || string[0] == 'X' ) ? 16 : 10);
    }
    return 0;
}

/* Translate HTML UTF8 marks to normal UTF8 (&#xFF; or e.g. &#123; -> char 123) */
char *unescape_html_UTF8(const char * data)
{
    char * result = NULL;
    if(data != NULL)
    {
        size_t i = 0, len = strlen(data);
        int tagflag = 0;
        int iB = 0;

        char * tag_open_ptr = NULL;
        result = g_malloc0(len+1);
        for (i = 0; i  < len; ++i)
        {
            char * semicol = NULL;
            if (data[i] == '&' && data[i+1] == '#' && (semicol = strstr(data+i,";")) != NULL)
            {
                int n = convert_to_number(&data[i+2]);

                if (n >= 0x800)
                {
                    result[iB++] = (char)(0xe0 | ((n >> 12) & 0x0f));
                    result[iB++] = (char)(0x80 | ((n >> 6 ) & 0x3f));
                    result[iB++] = (char)(0x80 | ((n      ) & 0x3f));
                }
                else if (n >= 0x80)
                {
                    result[iB++] = (char)(0xc0 | ((n >> 6) & 0x1f));
                    result[iB++] = (char)(0x80 | ((n     ) & 0x3f));
                }
                else
                {
                    result[iB++] = (char)n;
                }
                i =  (int)(semicol-data);
            }
            else /* normal char */
            {
                if (data[i] == '<')
                {
                    tag_open_ptr = (char*)&data[i+1];
                    tagflag = 1;
                    continue;
                }

                if(tagflag ==  0 )
                {
                    result[iB++] = data[i];
                }
                else if (data[i] == '>')
                {
                    if(tag_open_ptr != NULL)
                    {
                        if(g_strstr_len(tag_open_ptr,7,"br") != NULL)
                        {
                            result[iB++] = '\n';
                        }
                    }
                    tagflag = 0;
                }
            }
        }
        result[iB] = 0;
    }
    return result;
}

/* ------------------------------------------------------------- */

/* List from http://stackoverflow.com/questions/1082162/how-to-unescape-html-in-c/1082191#1082191,
 * Thanks for this. Probably directly from wikipedia: https://secure.wikimedia.org/wikipedia/en/wiki/List_of_XML_and_HTML_character_entity_references
 * I don't know a way how this can be done easier. But it's working well, so I guess I just let it be...
 */
const char * html_to_unicode_table[][2] =
{
    { "AElig", "Æ" },
    { "Aacute", "Á" },
    { "Acirc", "Â" },
    { "Agrave", "À" },
    { "Alpha", "Α" },
    { "Aring", "Å" },
    { "Atilde", "Ã" },
    { "Auml", "Ä" },
    { "Beta", "Β" },
    { "Ccedil", "Ç" },
    { "Chi", "Χ" },
    { "Dagger", "‡" },
    { "Delta", "Δ" },
    { "ETH", "Ð" },
    { "Eacute", "É" },
    { "Ecirc", "Ê" },
    { "Egrave", "È" },
    { "Epsilon", "Ε" },
    { "Eta", "Η" },
    { "Euml", "Ë" },
    { "Gamma", "Γ" },
    { "Iacute", "Í" },
    { "Icirc", "Î" },
    { "Igrave", "Ì" },
    { "Iota", "Ι" },
    { "Iuml", "Ï" },
    { "Kappa", "Κ" },
    { "Lambda", "Λ" },
    { "Mu", "Μ" },
    { "Ntilde", "Ñ" },
    { "Nu", "Ν" },
    { "OElig", "Œ" },
    { "Oacute", "Ó" },
    { "Ocirc", "Ô" },
    { "Ograve", "Ò" },
    { "Omega", "Ω" },
    { "Omicron", "Ο" },
    { "Oslash", "Ø" },
    { "Otilde", "Õ" },
    { "Ouml", "Ö" },
    { "Phi", "Φ" },
    { "Pi", "Π" },
    { "Prime", "″" },
    { "Psi", "Ψ" },
    { "Rho", "Ρ" },
    { "Scaron", "Š" },
    { "Sigma", "Σ" },
    { "THORN", "Þ" },
    { "Tau", "Τ" },
    { "Theta", "Θ" },
    { "Uacute", "Ú" },
    { "Ucirc", "Û" },
    { "Ugrave", "Ù" },
    { "Upsilon", "Υ" },
    { "Uuml", "Ü" },
    { "Xi", "Ξ" },
    { "Yacute", "Ý" },
    { "Yuml", "Ÿ" },
    { "Zeta", "Ζ" },
    { "aacute", "á" },
    { "acirc", "â" },
    { "acute", "´" },
    { "aelig", "æ" },
    { "agrave", "à" },
    { "alefsym", "ℵ" },
    { "alpha", "α" },
    { "amp", "&" },
    { "and", "∧" },
    { "ang", "∠" },
    { "apos", "'" },
    { "aring", "å" },
    { "asymp", "≈" },
    { "atilde", "ã" },
    { "auml", "ä" },
    { "bdquo", "„" },
    { "beta", "β" },
    { "brvbar", "¦" },
    { "bull", "•" },
    { "cap", "∩" },
    { "ccedil", "ç" },
    { "cedil", "¸" },
    { "cent", "¢" },
    { "chi", "χ" },
    { "circ", "ˆ" },
    { "clubs", "♣" },
    { "cong", "≅" },
    { "copy", "©" },
    { "crarr", "↵" },
    { "cup", "∪" },
    { "curren", "¤" },
    { "dArr", "⇓" },
    { "dagger", "†" },
    { "darr", "↓" },
    { "deg", "°" },
    { "delta", "δ" },
    { "diams", "♦" },
    { "divide", "÷" },
    { "eacute", "é" },
    { "ecirc", "ê" },
    { "egrave", "è" },
    { "empty", "∅" },
    { "emsp", " " },
    { "ensp", " " },
    { "epsilon", "ε" },
    { "equiv", "≡" },
    { "eta", "η" },
    { "eth", "ð" },
    { "euml", "ë" },
    { "euro", "€" },
    { "exist", "∃" },
    { "fnof", "ƒ" },
    { "forall", "∀" },
    { "frac12", "½" },
    { "frac14", "¼" },
    { "frac34", "¾" },
    { "frasl", "⁄" },
    { "gamma", "γ" },
    { "ge", "≥" },
    { "gt", ">" },
    { "hArr", "⇔" },
    { "harr", "↔" },
    { "hearts", "♥" },
    { "hellip", "…" },
    { "iacute", "í" },
    { "icirc", "î" },
    { "iexcl", "¡" },
    { "igrave", "ì" },
    { "image", "ℑ" },
    { "infin", "∞" },
    { "int", "∫" },
    { "iota", "ι" },
    { "iquest", "¿" },
    { "isin", "∈" },
    { "iuml", "ï" },
    { "kappa", "κ" },
    { "lArr", "⇐" },
    { "lambda", "λ" },
    { "lang", "〈" },
    { "laquo", "«" },
    { "larr", "←" },
    { "lceil", "⌈" },
    { "ldquo", "“" },
    { "le", "≤" },
    { "lfloor", "⌊" },
    { "lowast", "∗" },
    { "loz", "◊" },
    { "lrm", "\xE2\x80\x8E" },
    { "lsaquo", "‹" },
    { "lsquo", "‘" },
    { "lt", "<" },
    { "macr", "¯" },
    { "mdash", "—" },
    { "micro", "µ" },
    { "middot", "·" },
    { "minus", "−" },
    { "mu", "μ" },
    { "nabla", "∇" },
    { "nbsp", " " },
    { "ndash", "–" },
    { "ne", "≠" },
    { "ni", "∋" },
    { "not", "¬" },
    { "notin", "∉" },
    { "nsub", "⊄" },
    { "ntilde", "ñ" },
    { "nu", "ν" },
    { "oacute", "ó" },
    { "ocirc", "ô" },
    { "oelig", "œ" },
    { "ograve", "ò" },
    { "oline", "‾" },
    { "omega", "ω" },
    { "omicron", "ο" },
    { "oplus", "⊕" },
    { "or", "∨" },
    { "ordf", "ª" },
    { "ordm", "º" },
    { "oslash", "ø" },
    { "otilde", "õ" },
    { "otimes", "⊗" },
    { "ouml", "ö" },
    { "para", "¶" },
    { "part", "∂" },
    { "permil", "‰" },
    { "perp", "⊥" },
    { "phi", "φ" },
    { "pi", "π" },
    { "piv", "ϖ" },
    { "plusmn", "±" },
    { "pound", "£" },
    { "prime", "′" },
    { "prod", "∏" },
    { "prop", "∝" },
    { "psi", "ψ" },
    { "quot", "\"" },
    { "rArr", "⇒" },
    { "radic", "√" },
    { "rang", "〉" },
    { "raquo", "»" },
    { "rarr", "→" },
    { "rceil", "⌉" },
    { "rdquo", "”" },
    { "real", "ℜ" },
    { "reg", "®" },
    { "rfloor", "⌋" },
    { "rho", "ρ" },
    { "rlm", "\xE2\x80\x8F" },
    { "rsaquo", "›" },
    { "rsquo", "’" },
    { "sbquo", "‚" },
    { "scaron", "š" },
    { "sdot", "⋅" },
    { "sect", "§" },
    { "shy", "\xC2\xAD" },
    { "sigma", "σ" },
    { "sigmaf", "ς" },
    { "sim", "∼" },
    { "spades", "♠" },
    { "sub", "⊂" },
    { "sube", "⊆" },
    { "sum", "∑" },
    { "sup", "⊃" },
    { "sup1", "¹" },
    { "sup2", "²" },
    { "sup3", "³" },
    { "supe", "⊇" },
    { "szlig", "ß" },
    { "tau", "τ" },
    { "there4", "∴" },
    { "theta", "θ" },
    { "thetasym", "ϑ" },
    { "thinsp", " " },
    { "thorn", "þ" },
    { "tilde", "˜" },
    { "times", "×" },
    { "trade", "™" },
    { "uArr", "⇑" },
    { "uacute", "ú" },
    { "uarr", "↑" },
    { "ucirc", "û" },
    { "ugrave", "ù" },
    { "uml", "¨" },
    { "upsih", "ϒ" },
    { "upsilon", "υ" },
    { "uuml", "ü" },
    { "weierp", "℘" },
    { "xi", "ξ" },
    { "yacute", "ý" },
    { "yen", "¥" },
    { "yuml", "ÿ" },
    { "zeta", "ζ" },
    { "zwj", "\xE2\x80\x8D" },
    { "zwnj", "\xE2\x80\x8C" },
    { NULL, NULL }
};

/* ------------------------------------------------------------- */

/* returns newly allocated string without unicode like expressions */
char * strip_html_unicode(const gchar * string)
{
    if(string == NULL)
        return NULL;

    // Total length, iterator and resultbuf
    gsize sR_len = strlen(string), sR_i = 0;
    gchar * sResult = g_malloc0(sR_len + 1);

    for(gsize aPos = 0; aPos < sR_len; aPos++)
    {
        // An ampersand might be a hint
        if(string[aPos] == '&')
        {
            gchar * semicolon = NULL;
            if( (semicolon  = strchr(string+aPos,';')) != NULL)
            {
                // The distance between '&' and ';'
                gsize diff = semicolon - (string+aPos);

                // Only translate codes shorter than 10 signs.
                if(diff > 0 && diff < 8)
                {
                    // copy that portion so we can find the translation
                    gchar cmp_buf[diff];
                    strncpy(cmp_buf, string + aPos + 1 ,diff-1);
                    cmp_buf[diff-1] = '\0';

                    // Now find the 'translation' of this code
                    // This is a bit slow for performance aware applications
                    // Glyr isn't because it has to wait for data from the internet most
                    // of the time. You might want to add some sort of 'Hash'
                    gsize iter = 0;
                    while( html_to_unicode_table[iter][0] != NULL )
                    {
                        if(html_to_unicode_table[iter][0] &&  !strcmp(cmp_buf,html_to_unicode_table[iter][0]))
                        {
                            break;
                        }
                        iter++;
                    }

                    // If nothing found we just copy it
                    if(html_to_unicode_table[iter][0] != NULL && html_to_unicode_table[iter][1] != NULL)
                    {
                        // Copy the translation to the string
                        gsize trans_len = strlen(html_to_unicode_table[iter][1]);
                        strncpy(sResult + sR_i, html_to_unicode_table[iter][1], trans_len);

                        // Overjump next bytes.
                        sR_i += trans_len;
                        aPos += diff;
                        continue;
                    }
                }
            }
        }

        // Plain strcpy most of the time..
        sResult[sR_i++] = string[aPos];
    }
    return sResult;
}

/* ------------------------------------------------------------- */

/*
 * Remove all characters between the start tag $start and endtag $end.
 * Works inplace by copying & moving parts of the string as needed.
 * Returns number of bytes the 0 bytes has moved backward.
 * "Hello <World>!" -> "Hello !"; returns 7;
 * This is only used for HTML tags, there might be utf8 characters,
 * being rendered with the same glyph as '<', but won't escaped.
 */
gsize remove_tags_from_string(gchar * string, gint length, gchar start, gchar end)
{
    gchar * tagEnd;
    gsize ctr = 0;
    if(string != NULL)
    {
        gsize Len = (length < 0) ? strlen(string) : (size_t)length;
        if(Len != 0)
        {
            for(gsize n = Len-1; n != 0; --n)
            {
                if(string[n] == start)
                {
                    if((tagEnd = strchr(string + n + 1,end)) != NULL)
                    {
                        gchar * Tpon = tagEnd + 1;
                        gsize tLen = Tpon - (string+n);
                        gsize rest = string + Len - tagEnd;

                        tLen = (tLen < rest) ? tLen : rest;
                        memcpy (string+n, Tpon, tLen);
                        memmove(Tpon,Tpon+tLen,rest-tLen);
                        ctr += tLen;
                    }
                }
            }
        }
    }
    return ctr;
}

/* ------------------------------------------------------------- */

static gchar * trim_in_text(gchar * string)
{
    gchar * buffer = NULL;
    if(string != NULL)
    {
        gsize str_len = strlen(string), buf_pos = 0;
        buffer = g_malloc0(str_len + 1);

        gsize space_ctr= 0;
        gsize lfeed_ctr = 0;
        for(gsize it = 0; it < str_len; it++)
        {
            gboolean is_space = isspace(string[it]);
            gboolean is_lfeed = !isblank(string[it]) && is_space;

            lfeed_ctr = (is_lfeed) ? lfeed_ctr + 1 : 0;
            space_ctr = (is_space) ? space_ctr + 1 : 0;

            if(space_ctr < 2 || lfeed_ctr)		
            {	
                buffer[buf_pos++] = string[it];
            }
        }
    }
    return buffer;
}

/* ------------------------------------------------------------- */

/* Beautify lyrics in general, by removing endline spaces, *
 * trimming everything and removing double newlines        */
gchar * beautify_string(const gchar * lyrics)
{
    gchar * result = NULL;

    /* Strip special html entities */
    gchar * strip = unescape_html_UTF8(lyrics);
    if(strip != NULL)
    {
        /* Now convert all unichars like &#[x]num; */
        gchar * unicode = strip_html_unicode(strip);

        if(unicode != NULL)
        {
            gsize Len = strlen(unicode);
            for(gsize i = 0; i < Len; i++)
            {
                gsize newline_ctr = 0;
                gsize j = i;
                for(; j < Len && (unicode[j] == '\n' || unicode[j] == '\r'); j++)
                {
                    if(newline_ctr % 3 == 0)
                    {
                        unicode[j] = '\n';
                    }
                    else
                    {
                        unicode[j] = ' ';
                    }
                    newline_ctr++;
                }
                i = j + 1;
            }

            remove_tags_from_string(unicode,Len,'<','>');
            // gchar * trimmed = trim_after_newline(unicode);
            gchar * trimmed = trim_in_text(unicode);
            g_free(unicode);

            if(trimmed && g_utf8_validate(trimmed,-1,NULL) == TRUE)
            {
                result = g_utf8_normalize(trimmed,-1,G_NORMALIZE_NFKC);
                g_free(trimmed);
            }
            else
            {
                result = trimmed;
            }
        }
        g_free(strip);
    }
    return result;
}


/* ------------------------------------------------------------- */

/* Does not necessarely handle unicode, just ascii */
void trim_copy(gchar *input, gchar *output)
{
    gchar *end = output;
    gchar c;

    /* skip spaces at start */
    while(input[0] && isspace(*input))
    {
        ++input;
    }

    /* copy the rest while remembering the last non-whitespace */
    while(input[0])
    {
        // copy character
        c = *(output++) = *(input++);

        /* if its not a whitespace, this *could* be the last character */
        if( !isspace(c) )
        {
            end = output;
        }
    }

    /* write the terminating zero after last non-whitespace */
    *end = 0;
}


/* ------------------------------------------------------------- */

void trim_inplace(gchar *s)
{
    trim_copy(s, s);
}

/* ------------------------------------------------------------- */

/* Returns new pointer */
gchar * trim_nocopy(gchar * s)
{
    gchar * start = s;
    gchar * end = NULL;

    /* skip spaces at start */
    while(*start && isspace(*start))
        ++start;

    /* iterate over the rest remebering last non-whitespace */
    char *i = start;
    while(*i)
    {
        if( !isspace(*(i++)) )
            end = i;
    }

    /* white the terminating zero after last non-whitespace */
    if (end !=NULL) *end = 0;

    return start;
}

/* ------------------------------------------------------------- */

/* Just copies the value from begin to start (excluding end[0]) */
gchar * copy_value(const gchar * begin, const gchar * end)
{
    if(begin && end)
    {
        gsize length = end - begin;
        gchar * buffer = g_malloc0(length+1);

        if(buffer != NULL)
        {
            strncpy(buffer,begin,length);
            buffer[length] = '\0';
        }
        return buffer;
    }
    return NULL;
}

/* ------------------------------------------------------------- */

void chomp_breakline(gchar * string)
{
    if(string != NULL)
    {
        gsize len = strlen(string);
        while(--len)
        {
            if(string[len] == '\n')
            {
                string[len] = '\0';
                if(len != 0 && string[len-1] == '\r')
                {
                    string[len-1] = '\0';
                }
                break;
            }
        }
    }
}

/* ------------------------------------------------------------- */

gchar * convert_charset(const gchar * string, gchar * from, gchar * to, gsize * new_size)
{
    gchar * conv_string = NULL;
    GIConv converter;
    if((converter = g_iconv_open(from,to)) != (GIConv)-1)
    {
        GError * err_step_one = NULL;
        conv_string = g_convert_with_iconv(string,-1,converter,NULL,new_size,&err_step_one);
        if(conv_string == NULL)
        {
            g_print("glyr: conversion-error: %s\n",err_step_one->message);
        }
        g_iconv_close(converter);
    }
    else
    {
        g_print("glyr: Unable to convert charsets.\n");
    }
    return conv_string;
}

/* ------------------------------------------------------------- */

gchar * get_search_value(gchar * ref, gchar * name, gchar * end_string)
{
    gchar * result = NULL;
    if(ref && name)
    {
        gchar * begin = strstr(ref,name);
        if(begin != NULL)
        {
            begin += strlen(name);
            result = copy_value(begin,strstr(begin,end_string));
        }
    }	
    return result;
}

/* ------------------------------------------------------------- */

/* Note: Not case-sens: Ä -> a! */
const gchar * const umlaut_table[][2] = {
    {"ä",  "a"},
    {"ü",  "u"},
    {"ö",  "o"},
    {"ß", "ss"},
    {"\\s","-"}
};

const gsize umlaut_table_size = sizeof(umlaut_table) / (2 * sizeof(gchar*));

/* Replaces umlauts like ä with an approx. like a */
gchar * translate_umlauts(gchar * string)
{
    gchar * result = NULL;
    if(string != NULL)
    {
        result = regex_replace_by_table(string,umlaut_table,umlaut_table_size);
    }
    return result;
}

/* Match string against a GRegex */
gboolean regex_match_compiled(const gchar * string, const GRegex * cRegex)
{
    gboolean retv = FALSE;
    if(string != NULL)
    {
        if(cRegex == NULL)
            return TRUE;

        retv = g_regex_match(cRegex, string, 0, NULL);
    }
    return retv;
}
