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
#ifndef STRINGOP_H
#define STRINGOP_H

#include <stdarg.h>

#define nextTag(X) while(*X && *X++ != '>')
int    levenshtein_strcmp(const char * s, const char * t);
char * escape_slashes(const char * in);
char * ascii_strdown_modify(char * string);
char * ascii_strdown (const char * string);
char * strreplace(const char * string, const char * subs, const char * with);
char * prepare_url   (const char * URL, const char * artist, const char * album, const char * title);
char * unescape_html_UTF8(const char *data);
int    strcrepl(char* in, const char orig, const char new);
char * strip_html_unicode(const char * string);
char * beautify_lyrics(const char * lyrics);
char * strdup_printf (const char *format, ...);
char * strrstr_len(char *haystack, char *needle, size_t len);
void trim_copy(char *input, char *output);
void trim_inplace(char *s);
char * trim_nocopy(char *s);
char * getStr (char ** s, char * start, char * end);
char * remove_html_tags_from_string(const char * string, size_t len);
char * copy_value(const char * begin, const char * end);
char * get_next_word(const char * string, const char * delim, size_t *offset, size_t len);
int x_vasprintf(char ** str, const char * fmt, va_list params);

#endif
