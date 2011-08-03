#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* libcurl header, link with -lcurl */
#include <curl/curl.h>

/* Notes on making this a seperate lib: *
 * - Copy stringlib.[ch] with it        *
 * - Link with libcurl 	                *
 * - copy download_single() from core.c *
 * - replace the apikey with yours.     */
#include "stringlib.h"
#include "types.h"
#include "core.h"

/* Module prototypes */
static const char * gtrans_get_trans_url(const char * input, const char * from, const char * to);
static const char * gtrans_get_detector_url(const char * input);
static char * json_get_single_value(char * sval, size_t keylen);
static char * gtrans_parse_trans_json(GlyMemCache * cache, const char * search_key);
static char * gtrans_parse_detector_json(GlyMemCache * cache, float * correctness);

/* ------------------------------------------------------------------- */

static const char * gtrans_get_trans_url(const char * input, const char * from, const char * to)
{
    char * resultURL = NULL;
    if(input != NULL)
    {
        char * escape_input = curl_easy_escape(NULL,input,0);
        if(escape_input != NULL)
        {
            resultURL = strdup_printf("https://www.googleapis.com/language/translate/v2?key=%s%s%s&target=%s&prettyprint=true&q=%s",
                                      API_KEY_GTRANS,
                                      (from==NULL) ? "" : "&source=", // &source field can be omitted, autodetect then.
                                      (from==NULL) ? "" : from,
                                      to,escape_input
                                     );
        }
        free(escape_input);
    }
    return resultURL;
}

/* ------------------------------------------------------------------- */

static const char * gtrans_get_detector_url(const char * input)
{
    char * resultURL = NULL;
    if(input != NULL)
    {
        char * escape_input = curl_easy_escape(NULL,input,0);
        if(escape_input != NULL)
        {
            resultURL = strdup_printf("https://www.googleapis.com/language/translate/v2/detect?key=%s&q=%s",API_KEY_GTRANS,escape_input);
        }
        free(escape_input);
    }
    return resultURL;
}

/* ------------------------------------------------------------------- */

static char * json_get_single_value(char * keyval, size_t keylen)
{
    char * result_val = NULL;
    char * p_curs = keyval + keylen;

    /* Get to the beginning of the value */
    while(*p_curs && *p_curs != ' ')
        p_curs++;

    if(*p_curs)
    {
        char * end_mark = (p_curs += (*p_curs == ' '));
        while(*end_mark && *end_mark != '\n' && *end_mark != '}'
                && *end_mark != ','  && *end_mark != ']')
            end_mark++;

        if(*end_mark)
        {
            result_val = copy_value(p_curs+1,end_mark-1);
        }
    }
    return result_val;
}

/* ------------------------------------------------------------------- */

static char * gtrans_parse_trans_json(GlyMemCache * cache, const char * search_key)
{
    char * resultBuf = NULL;
    if(cache != NULL)
    {
        char * p_cursor = cache->data;
        if(p_cursor != NULL)
        {
            // Brute force approach actually (but works! =))
            if((p_cursor = strstr(p_cursor,search_key)) != NULL)
            {
                char *end_mark = NULL;

                p_cursor += strlen(search_key);
                while(*p_cursor && *p_cursor != '"')
                    p_cursor++;
                if(*p_cursor == '"')
                {
                    p_cursor++;
                    end_mark = strchr(p_cursor,'"');
                }

                if(end_mark != NULL)
                {
                    while(end_mark != p_cursor && *end_mark != '"')
                        end_mark--;

                    resultBuf = copy_value(p_cursor,end_mark);
                    if(resultBuf != NULL)
                    {
                        // Remove silly marks
                        while(*resultBuf == '"')
                            resultBuf++;
                    }
                }
            }
        }
    }
    return resultBuf;
}

/* ------------------------------------------------------------------- */

#define LANG_KEY "\"language\":"
#define CONF_KEY "\"confidence\":"

// for gtrans_parse_json
#define DETT_KEY "\"detectedSourceLanguage\":"
#define TRNS_KEY "\"translatedText\":"


static char * gtrans_parse_detector_json(GlyMemCache * cache, float * correctness)
{
    char * resultBuf = NULL;
    if(cache && cache->data)
    {
        resultBuf = json_get_single_value(strstr(cache->data,LANG_KEY),strlen(LANG_KEY));
        char * v_conf = json_get_single_value(strstr(cache->data,CONF_KEY),strlen(CONF_KEY));

        if(v_conf != NULL && correctness)
        {
            *correctness = atof(v_conf);
        }
    }
    return resultBuf;
}

/* ------------------------------------------------------------------- */

/* Max size of bytes google can return for some reason */
#define BLOCK_BUF 500
#define NEWLINE_MARKUP "__"
/* Modifies $to_translate with the translation */
void Gly_gtrans_translate(GlyQuery * s, GlyMemCache * to_translate)
{
    /* BROKEN */
#if 0
    char * concat_chain = NULL;
    char block_buf[BLOCK_BUF+2];

    if(s && to_translate && s->gtrans.target)
    {

        // so yesterday I translated an image. O RLY?
        if(to_translate->is_image)
            return;

        // no need to translate, eh?
        if(s->gtrans.source && !strcmp(s->gtrans.target,s->gtrans.source))
            return;

        // Split into blocks
        size_t offset = 0;
        size_t copysz = BLOCK_BUF;
        while(offset < to_translate->size)
        {

            memset(block_buf,0,BLOCK_BUF+2);

            // Cut off if not enough left
            // This had a nasty bug, as previously no int casting was done
            if((int)offset >= ((int)to_translate->size - BLOCK_BUF))
            {
                copysz = to_translate->size - offset - 1;
            }

            // Copy block
            memcpy(block_buf, to_translate->data + offset,copysz);
            block_buf[copysz] = '\0';
            offset += BLOCK_BUF;

            // Escape those silly newlines
            char * escaped_newlines = strreplace(block_buf,"\n",NEWLINE_MARKUP);
            if(escaped_newlines != NULL)
            {

                // get the url we need to download
                const char * dl_url = gtrans_get_trans_url(escaped_newlines,s->gtrans.source, s->gtrans.target);
                if(dl_url)
                {
                    // Download and parse
                    GlyMemCache * DCache = NULL;
                    char * parsed_json = gtrans_parse_trans_json((DCache=download_single(dl_url,s,NULL)), (s->gtrans.source == NULL) ? TRNS_KEY : TRNS_KEY);

                    // Not needed anymore
                    if(DCache != NULL)
                        DL_free(DCache);

                    if(parsed_json != NULL)
                    {
                        // Not the actual purpose of it, but well..
                        char * back_up = concat_chain;
                        concat_chain = strdup_printf("%s%s",(concat_chain) ? concat_chain : "",parsed_json);
                        if(back_up) free(back_up);

                        // Free previous
                        free(parsed_json);
                        parsed_json = NULL;
                    }

                    // Free bufs
                    free((char*)dl_url);
                }
            }
            free(escaped_newlines);
            escaped_newlines = NULL;

        }


        /* I just found out I can do this.. */
        {
            char * back_ptr = concat_chain;
            if(concat_chain != NULL)
            {
                char * temp = strreplace(concat_chain,NEWLINE_MARKUP,"\n");
                free(back_ptr);
                back_ptr = NULL;

                if(temp)
                {
                    concat_chain = beautify_lyrics(temp);
                    free(temp);
                    temp = NULL;
                }


            }
        }

        if(concat_chain != NULL)
        {
            if(to_translate->data != NULL)
            {
                free(to_translate->data);
            }
            to_translate->data = concat_chain;
            to_translate->size = strlen(concat_chain);
        }
    }
#endif
}

/* ------------------------------------------------------------------- */

/* Ask google translator what language we are dealing with, NULL if unknown */
char * Gly_gtrans_lookup(GlyQuery * s, const char * snippet, float * correctness)
{
    char * result_lang = NULL;
    if(s  && snippet)
    {
        const char * dl_url = gtrans_get_detector_url(snippet);
        if(dl_url != NULL)
        {
            GlyMemCache * DCache = download_single(dl_url,s,NULL);
            if(DCache != NULL)
            {
                result_lang = gtrans_parse_detector_json(DCache,correctness);
                DL_free(DCache);
            }
            free((char*)dl_url);
        }
    }
    return result_lang;
}

/* ------------------------------------------------------------------- */

char ** Gly_gtrans_list(GlyQuery * s)
{
    char ** result_list = NULL;
    if(s != NULL)
    {
        size_t elem_counter = 0;
        const char * dl_url = "https://www.googleapis.com/language/translate/v2/languages?key="API_KEY_GTRANS"&prettyprint=true";
        if(dl_url != NULL)
        {
            GlyMemCache * DCache = download_single(dl_url,s,NULL);
            if(DCache != NULL && DCache->data != NULL)
            {
                char * lang_elem = DCache->data, * value;
                const size_t lang_keylen = strlen(LANG_KEY);

                // get all langs
                while((lang_elem = strstr(lang_elem+1,LANG_KEY)))
                {
                    if( (value = json_get_single_value(lang_elem,lang_keylen)) )
                    {
                        // add this lang

                        result_list = realloc(result_list, (elem_counter + 2) * sizeof(char*));
                        result_list[elem_counter++] = value; // Just refernce that value
                        result_list[elem_counter  ] = NULL;  // make sure the list is always NULL terminated
                    }
                }
                DL_free(DCache);
            }
        }
    }
    return result_list;
}

/* ------------------------------------------------------------------- */
