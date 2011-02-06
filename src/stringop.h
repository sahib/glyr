#ifndef STRINGOP_H
#define STRINGOP_H

int    levenshtein_strcmp(const char * s, const char * t);
char * escape_slashes(const char * in);
char * ascii_strdown (const char * string, size_t len);
char * ascii_strdown_modify(char * string, size_t len);
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
char *trim_nocopy(char *s);
char * getStr (char ** s, char * start, char * end);

#endif
