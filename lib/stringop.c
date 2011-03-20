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

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>

// For curl_easy_escape()
#include <curl/curl.h>

#include "stringop.h"
#include "types.h"

// Implementation of the Levenshtein distance algorithm
// Compare the number of edits needed to convert string $s
// to target string $t
// This is used to correct typos in e.g: lyricswiki
// If you're a member of lyricswiki:
// STOP MAKING TYPOS! IT TOOK ME OVER AN HOUR TO WRITE THIS! :-P
// (not meant to be serious, you guys are cool ;-))
// For instructions go to http://www.merriampark.com/ld.htm
// They have also Source code in C++, Java & VB

size_t levenshtein_strcmp(const char * s, const char * t)
{
    int n = (s) ? strlen(s)+1 : 0;
    int m = (t) ? strlen(t)+1 : 0;

    // Nothing to compute really..
    if (n==0)
    {
        return m;
    }
    if (m==0)
    {
        return n;
    }

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
            if (a < b)
            {
                if (a < c)
                {
                    d[i][j]=a;
                }
                else
                {
                    d[i][j]=c;
                }
            }
            else
            {
                if (b < c)
                {
                    d[i][j]=b;
                }
                else
                {
                    d[i][j]=c;
                }
            }
        }
    }

    // The result is stored in the very right down cell
    return d[n-1][m-1];
}

/* ------------------------------------------------------------- */

char * ascii_strdown(const char * string)
{
    return (char*)((string) ? ascii_strdown_modify(strdup(string)) : NULL);
}

/* ------------------------------------------------------------- */

// Like ascii_strdown, but does modify buffer in place
char * ascii_strdown_modify(char * string)
{
    if(string)
    {
        size_t i = 0,len = strlen(string);
        for(i = 0; i < len; i++)
            string[i] = tolower(string[i]);
    }
    return string;
}

/* ------------------------------------------------------------- */

// recursively replace strings
static char * __strsubs(char * string, const char * subs, size_t subs_len, const char * with, size_t with_len, long offset)
{
    /* Terminate when no occurences left */
    char * occ = NULL;
    if( (occ = strstr(string+offset,subs)) != NULL)
    {
        /* string has a variable length */
        size_t strn_len = strlen(string);
        char * new = calloc(strn_len + with_len - subs_len + 1,sizeof(char));

        /* Split & copy */
        strncat(new, string, occ-string);
        strncat(new, with, with_len);
        strncat(new, occ+subs_len, strn_len - subs_len - (occ-string));

        /* free previous pointer */
        free(string);

        /* Call recursively on next possible occurence */
        return __strsubs(new,subs,subs_len,with,with_len,(occ-string)+with_len);
    }
    return string;
}

/* ------------------------------------------------------------- */

/* Return always a newly allocated string - wraps around __strsubs() */
char * strreplace(const char * string, const char * subs, const char * with)
{
    size_t subs_len, with_len;

    /* Handle special cases (where __strsubs would return weird things) */
    if(string && *string)
    {
        if(subs && *subs)
        {
            /* Call strlen() only once */
            subs_len = strlen(subs);
            with_len = (with) ? strlen(with) : 0;

            /* Replace all occurenced recursevely */
            return __strsubs(strdup(string), subs, subs_len, with, with_len,0);
        }
        return strdup(string);
    }
    return NULL;
}


/* ------------------------------------------------------------- */

// Replace a single character
int strcrepl(char* in, const char orig, const char new)
{
    size_t i = 0;
    for (; in[i]; i++)
        if (in[i] == orig)
            in[i] = new;

    return i;
}


/* ------------------------------------------------------------- */

// Remove anything in between '(' && ')'
static char * remove_surrender(const char * name)
{
    if (name == NULL) return NULL;

    size_t len = strlen(name), i = 0, lock = 0, j = 0;
    char  *ret = malloc(len+1);

    for (; i < len; i++)
    {
        if (name[i] == '(' )
        {
            lock = 1;
            continue;
        }
        if (name[i] == ')' )
        {
            lock = 0;
            continue;
        }
        if (name[i] != '\t' && !lock)
        {
            ret[j++] = name[i];
        }
    }

    ret[len] = '\0';
    return ret;
}

/* ------------------------------------------------------------- */

static char * prepare_string(const char * input)
{
    char * result = NULL;
    if(input != NULL)
    {
        char * downed = ascii_strdown(input);
        if(downed)
        {
            char * escaped = curl_easy_escape(NULL,downed,0);
            if(escaped)
            {
                result = remove_surrender(escaped);
                free(escaped);
            }
            free(downed);
        }
    }
    return result;
}

static void swap_string(char ** tmp, const char * subs, const char * with)
{
    char * swap = *tmp;
    *tmp = strreplace(swap,subs,with);

    if(swap)
    {
        free(swap);
    }
}

// Prepares the url for you to get downloaded. You don't have to call this.
char * prepare_url(const char *URL, const char *artist, const char *album, const char *title)
{
    char * tmp = NULL;
    if(URL != NULL)
    {
        tmp = strdup(URL);

        char * p_artist = prepare_string(artist);
        char * p_album  = prepare_string(album);
        char * p_title  = prepare_string(title);

        swap_string(&tmp,"%artist%",p_artist);
        swap_string(&tmp,"%album%", p_album);
        swap_string(&tmp,"%title%", p_title);

        if(p_artist)
            free(p_artist);
        if(p_album)
            free(p_album);
        if(p_title)
            free(p_title);
    }
    return tmp;
}

/* ------------------------------------------------------------- */

char * get_next_word(const char * string, const char * delim, size_t *offset, size_t len)
{
    char * word = NULL;
    if(string && delim && *(offset) < len)
    {
        char * occurence = strstr(string+(*offset),delim);
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
        char * trim = calloc(len+1,sizeof(char));
        if(trim)
        {
            trim_copy(word,trim);
        }
        free(word);
        word = trim;
    }
    return word;
}

/* ------------------------------------------------------------- */

// find the last occurrance of find in string
// not the fastest implementation, but string
// manipulation is not the bottleneck here...
char * strrstr_len(char *haystack, char *needle, size_t len)
{
    size_t stringlen, findlen, lc = 0;
    char *cp = NULL;

    if(haystack && needle)
    {
        findlen = strlen(needle);
        stringlen = strlen(haystack);

        if (findlen > stringlen)
            return NULL;

        for (cp = haystack + stringlen - findlen; cp >= haystack && lc <= len; cp--,lc++)
        {
            if (strncmp(cp, needle, findlen) == 0)
            {
                return cp;
            }
        }
    }
    return cp;
}


/* ------------------------------------------------------------- */

/* Translate strings of &ouml; (== ü) to characters */
char *unescape_html_UTF8(const char * data)
{
    char * result = NULL;

    if(data != NULL)
    {
        size_t i = 0, len = strlen(data);
        int tagflag = 0;
        int iB = 0;

        result = malloc(len+1);
        memset(result,0,len);

        for (i = 0; i  < len; ++i)
        {
            char * semicol = NULL;
            if (data[i] == '&' && data[i+1] == '#' && (semicol = strstr(data+i,";")) != NULL)
            {
                int n = atoi(&data[i+2]);

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
                i =  (int)ABS(semicol-data);
            }
            else
            {
                if (data[i] == '<')
                {
                    tagflag = 1;
                    continue;
                }
                if (tagflag ==  0 ) result[iB++] = data[i];
                else if (data[i] == '>') result[iB++] = '\n';

                if (data[i] == '>') tagflag = 0;
            }
        }
        result[iB] = 0;
    }
    return result;
}

/* ------------------------------------------------------------- */

/* replacement for the non-portable vasprintf() */
int x_vasprintf(char ** str, const char * fmt, va_list params)
{
    int r = -1;
    if(!str)
    {
        return r;
    }

#ifndef WIN32
    /* Assume we have a vasprintf() somewhere */
    r = vasprintf (str, fmt, params);
#else
    /* Dirty hack to get the length of the fmt string */
    /* Windows does sadly not offer vasprintf(),      */
    /* So, the string is printed to a NUL device, and */
    /* characters are counted and saved in buf_len    */
    /* The buffer is allocated and built by vsnprintf */
    FILE * nul = fopen("NUL","w");
    if(nul)
    {
        size_t buf_size = vfprintf(nul, fmt, params) + 1;
        char * buffer   = calloc(buf_size+1,sizeof(char));

        if(buffer)
        {
            r = vsnprintf(buffer, buf_size, fmt, params);
            *str = buffer;
        }
        fclose(nul);
    }
#endif
    return r;
}

/* ------------------------------------------------------------- */

char *strdup_printf (const char *format, ...)
{
    if(format)
    {
        va_list arg;
        char *tmp = NULL;

        va_start (arg, format);
        if(x_vasprintf (&tmp, format, arg) == -1)
        {
            return NULL;
        }
        va_end (arg);
        return tmp;
    }
    return NULL;
}

/* ------------------------------------------------------------- */

const char *umap[][2] =
{
    {"#x80","€"},
    {"#x81"," "},
    {"#x82","‚"},
    {"#x83","ƒ"},
    {"#x84","„"},
    {"#x85","…"},
    {"#x86","†"},
    {"#x87","‡"},
    {"#x88","ˆ"},
    {"#x89","‰"},
    {"#x8A","Š"},
    {"#x89","‰"},
    {"#x8C","Œ"},
    {"#x8D","‹"},
    {"#x8E","Ž"},
    {"#x8F"," "},
    {"#x90"," "},
    {"#x91","‘"},
    {"#x92","’"},
    {"#x93","“"},
    {"#x94","”"},
    {"#x95","•"},
    {"#x96","–"},
    {"#x97","—"},
    {"#x98","˜"},
    {"#x99","™"},
    {"#x9A","š"},
    {"#x9B","›"},
    {"#x9C","œ"},
    {"#x9D"," "},
    {"#x9E","ž"},
    {"#x9F","Ÿ"},
    {"nbsp",""},
    {"iexcl","¡"},
    {"cent","¢"},
    {"pound","£"},
    {"curren","¤"},
    {"yen","¥"},
    {"brvbar","¦"},
    {"sect","§"},
    {"uml","¨"},
    {"amp","&"},
    {"copy","©"},
    {"ordf","ª"},
    {"laquo","«"},
    {"not","¬"},
    {"shy","­"},
    {"reg","®"},
    {"macr","¯"},
    {"deg","°"},
    {"plusmn","±"},
    {"sup2","²"},
    {"sup3","³"},
    {"acute","´"},
    {"micro","µ"},
    {"para","¶"},
    {"middot","·"},
    {"cedil","¸"},
    {"sup1","¹"},
    {"ordm","º"},
    {"raquo","»"},
    {"frac14","¼"},
    {"frac12","½"},
    {"frac34","¾"},
    {"iquest","¿"},
    {"Agrave","À"},
    {"Aacute","Á"},
    {"Acirc","Â"},
    {"Atilde","Ã"},
    {"Auml","Ä"},
    {"Aring","Å"},
    {"AElig","Æ"},
    {"Ccedil","Ç"},
    {"Egrave","È"},
    {"Eacute","É"},
    {"Ecirc","Ê"},
    {"Euml","Ë"},
    {"Igrave","Ì"},
    {"Iacute","Í"},
    {"Icirc","Î"},
    {"Iuml","Ï"},
    {"ETH","Ð"},
    {"Ntilde","Ñ"},
    {"Ograve","Ò"},
    {"Oacute","Ó"},
    {"Ocirc","Ô"},
    {"Otilde","Õ"},
    {"Ouml","Ö"},
    {"times","×"},
    {"Oslash","Ø"},
    {"Ugrave","Ù"},
    {"Uacute","Ú"},
    {"Ucirc","Û"},
    {"Uuml","Ü"},
    {"Yacute","Ý"},
    {"THORN","Þ"},
    {"szlig","ß"},
    {"agrave","à"},
    {"aacute","á"},
    {"acirc","â"},
    {"atilde","ã"},
    {"auml","ä"},
    {"aring","å"},
    {"aelig","æ"},
    {"ccedil","ç"},
    {"egrave","è"},
    {"eacute","é"},
    {"ecirc","ê"},
    {"euml","ë"},
    {"quot","\""},
    {"igrave","ì"},
    {"iacute","í"},
    {"icirc","î"},
    {"iuml","ï"},
    {"eth","ð"},
    {"ntilde","ñ"},
    {"ograve","ò"},
    {"oacute","ó"},
    {"ocirc","ô"},
    {"otilde","õ"},
    {"ouml","ö"},
    {"divide","÷"},
    {"oslash","ø"},
    {"ugrave","ù"},
    {"uacute","ú"},
    {"ucirc","û"},
    {"uuml","ü"},
    {"yacute","ý"},
    {"thorn","þ"},
    {"yuml","ÿ"},
    {"#x2500","─"},
    {"#x2502","│"},
    {"#x250C","┌"},
    {"#x2510","┐"},
    {"#x2514","└"},
    {"#x2518","┘"},
    {"#x251C","├"},
    {"#x2524","┤"},
    {"#x252C","┬"},
    {"#x2534","┴"},
    {"#x253C","┼"},
    {"#x3B1","α"},
    {"#x3B2","β"},
    {"#x3B3","γ"},
    {"#x394","Δ"},
    {"#x2211","∑"},
    {"#x221A","√"},
    {"#x222B","∫"},
    {"#x2248","≈"},
    {"#x2260","≠"},
    {"#x2264","≤"},
    {"#x2265","≥"},
    {"#x2190","←"},
    {"#x2191","↑"},
    {"#x2192","→"},
    {"#x2193","↓"},
    {"#x2194","↔"},
    {"#x2195","↕"},
    {"#x2022","•"},
    {"#x2044","⁄"},
    {"#x2030","‰"},
    {"#x2122","™"},
    {"#x20AC","€"},
    {NULL, NULL  }
};

/* ------------------------------------------------------------- */

/* returns newly allocated string without unicode like expressions */
char * strip_html_unicode(const char * string)
{
    if (string == NULL)
        return NULL;

    // Total length, iterator and resultbuf
    size_t sR_len = strlen(string), sR_i = 0;
    char * sResult = calloc(sR_len+1,sizeof(char));

    size_t aPos = 0;
    for(   aPos = 0; aPos < sR_len; aPos++)
    {
        // An ampersand might be a hint
        if(string[aPos] == '&')
        {
            char * semicolon = NULL;
            if(  (semicolon  = strchr(string+aPos,';')) != NULL)
            {
                // The distance between '&' and ';'
                size_t diff = semicolon - (string+aPos);

                // Only translate codes shorter than 10 signs.
                if(diff > 0 && diff < 8)
                {
                    // copy that portion so we can find the translation
                    char cmp_buf[diff];
                    strncpy(cmp_buf, string + aPos + 1 ,diff-1);
                    cmp_buf[diff-1] = '\0';

                    // Now find the 'translation' of this code
                    // This is a bit slow for performance aware applications
                    // Glyr isn't because it has to wait for data from the internet most
                    // of the time. You might want to add some sort of 'Hash'
                    size_t iter = 0;
                    while( umap[iter][0] != NULL )
                    {
                        if(umap[iter][0] &&  !strcmp(cmp_buf,umap[iter][0]))
                        {
                            break;
                        }
                        iter++;
                    }

                    // If nothing found we just copy it
                    if(umap[iter][0] != NULL && umap[iter][1] != NULL)
                    {
                        // Copy the translation to the string
                        size_t trans_len = strlen(umap[iter][1]);
                        strncpy(sResult + sR_i, umap[iter][1], trans_len);

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
 */
size_t remove_tags_from_string(char * string, int length, char start, char end)
{
     char *tagEnd;
     size_t ctr = 0;
     if(string != NULL)
     { 
	size_t n, L = (length < 0) ? strlen(string) : (size_t)length;
	for(n = L-1; n; --n)
	{
	    if(string[n] == start)
	    {
		if( (tagEnd = strchr(string+n+1,end)) )
		{
		    char * Tpon = tagEnd + 1; 
		    size_t tLen = Tpon - (string+n);
		    size_t rest = string + L - tagEnd;

		    tLen = (tLen < rest) ? tLen : rest;
		    memcpy (string+n, Tpon, tLen);
   		    memmove(Tpon,Tpon+tLen,rest-tLen);

		    ctr += tLen;
		}
	    } 
	}
     }
     return ctr;
}

/* ------------------------------------------------------------- */

/* Beautify lyrics in general, by removing endline spaces, *
 * trimming everything and removing double newlines        */
char * beautify_lyrics(const char * lyrics)
{
    char * result = NULL;

	// Strip numeric HTML unicodes 
    char * strip = strip_html_unicode(lyrics);
    if(strip)
    {
        /* Now convert all unichars like &ouml; */
        char * unicode = unescape_html_UTF8(strip);
        free(strip);

        if(unicode)
        {
            size_t i,j,len   = strlen(unicode);
            for(i = 0; i < len; i++)
            {
                int tlen = 0;
                for(j = i; j < len && unicode[j] == '\n'; j++)
                {
                    ++tlen;
                    if(tlen > 1)
                    {
                        unicode[j] = ' ';
                    }
                }
                if(tlen > 1)
                    unicode[j-1] = '\n';

                i = j+1;
            }

            len -= remove_tags_from_string(unicode,len,'<','>');
			char * trimd = calloc(1,sizeof(char) * (len+1));
			if(trimd)
			{
				trim_copy(unicode,trimd);
				free(unicode);
			}

			result = trimd;
        }
    }
    return result;
}


/* ------------------------------------------------------------- */

void trim_copy(char *input, char *output)
{
    char *end = output;
    char c;

    // skip spaces at start
    while(*input && isspace(*input))
        ++input;

    // copy the rest while remembering the last non-whitespace
    while(*input)
    {
        // copy character
        c = *(output++) = *(input++);

        // if its not a whitespace, this *could* be the last character
        if( !isspace(c) )
            end = output;
    }

    // write the terminating zero after last non-whitespace
    *end = 0;
}


/* ------------------------------------------------------------- */

void trim_inplace(char *s)
{
    trim_copy(s, s);
}


/* ------------------------------------------------------------- */

char *trim_nocopy(char *s)
{
    char *start = s;
    char *end = NULL;
    // skip spaces at start
    while(*start && isspace(*start))
        ++start;

    char *i = start;
    // iterate over the rest remebering last non-whitespace
    while(*i)
    {
        if( !isspace(*(i++)) )
            end = i;
    }

    // white the terminating zero after last non-whitespace
    if (end !=NULL) *end = 0;

    return start;
}

/* ------------------------------------------------------------- */

char * copy_value(const char * begin, const char * end)
{
    if(begin && end)
    {
        size_t length = end - begin;
        char * buffer = malloc(length+1);
        if(buffer)
        {
            strncpy(buffer,begin,length);
            buffer[length] = '\0';
        }
        return buffer;
    }
    return NULL;
}

/* ------------------------------------------------------------- */

char * getStr (char ** s, char * start, char * end)
{
    char * tmp = start;

    start = strstr(*s, start);
    end = strstr(*s, end);

    if (start == NULL || end == NULL)
    {
        return NULL;
    }

    *(end) = 0;
    *(s) = end+1;

    return start+strlen(tmp);
}

/* ------------------------------------------------------------- */
