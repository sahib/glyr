#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <check.h>
#include <glib.h>

#include "../../lib/glyr.h"

static void setup(GlyrQuery * q, GLYR_GET_TYPE type, int num)
{
    glyr_init();
    atexit(glyr_cleanup);

    glyr_query_init(q);
    glyr_opt_artist(q,"Equilibrium");
    glyr_opt_album(q, "Sagas");
    glyr_opt_title(q, "Wurzelbert");
    glyr_opt_type(q, type);
    glyr_opt_number(q,num);
    glyr_opt_verbosity(q,0);
}

//--------------------

static void unsetup(GlyrQuery * q, GlyrMemCache * list)
{
    glyr_free_list(list);
    glyr_query_destroy(q);
}

//--------------------
//--------------------

START_TEST(test_glyr_init)
{
  GlyrQuery q;
  fail_unless(glyr_get(NULL,NULL,NULL) == NULL,"glyr_get() should return NULL without INIT");
  fail_unless(glyr_get(&q,NULL,NULL) == NULL,"glyr_get() should still return NULL, even with uninitalized query.");

  glyr_init();
  fail_unless(glyr_get(NULL,NULL,NULL) == NULL,"glyr_get() should return NULL without INIT");
  fail_unless(glyr_get(&q,NULL,NULL) == NULL,"should not access bad memory");

  glyr_init();
  fail_unless(glyr_get(NULL,NULL,NULL) == NULL,"glyr_get() should return NULL without INIT");
  fail_unless(glyr_get(&q,NULL,NULL) == NULL,"should not access bad memory");

  glyr_cleanup();
  glyr_cleanup();
  fail_unless(glyr_get(NULL,NULL,NULL) == NULL,"glyr_get() should return NULL without INIT");
  fail_unless(glyr_get(&q,NULL,NULL) == NULL,"should not access bad memory");

  glyr_query_init(&q);
  fail_unless(q.is_initalized == 0xDEADBEEF,NULL);
  fail_unless(glyr_get(&q,NULL,NULL) == NULL,"should not access bad memory");
  glyr_query_destroy(&q);
}
END_TEST

//--------------------

START_TEST(test_glyr_destroy_before_init)
{
    glyr_cleanup();
    glyr_init();
}
END_TEST


//--------------------

START_TEST(test_glyr_query_init)
{
    GlyrQuery q;

    /* Crash? */
    glyr_query_init(NULL);

    glyr_query_init(&q);
    fail_unless(q.is_initalized == 0xDEADBEEF,NULL);

    GlyrQuery * alloc = malloc(sizeof(GlyrQuery));
    glyr_query_init(alloc);
    fail_unless(alloc->is_initalized == 0xDEADBEEF,NULL);

    glyr_query_destroy(&q);
}
END_TEST

//--------------------

START_TEST(test_glyr_cache_free)
{
    GlyrMemCache * test = glyr_cache_new();
    glyr_cache_free(NULL);
    glyr_cache_free(test);
}
END_TEST

//--------------------

START_TEST(test_glyr_cache_copy)
{
    GlyrMemCache * test = glyr_cache_new();
    glyr_cache_set_data(test,g_strdup("some data"),-1);
    test->next = (GlyrMemCache*)0x1010;
    test->prev = (GlyrMemCache*)0x0101;

    GlyrMemCache * copy = glyr_cache_copy(test);
    fail_unless(memcmp(copy->data,test->data,test->size) == 0,"Should have the same data");
    fail_unless(copy->next == NULL,NULL);
    fail_unless(copy->prev == NULL,NULL);

    glyr_cache_free(copy);
    glyr_cache_free(test);
}
END_TEST

//--------------------

START_TEST(test_glyr_query_destroy)
{
    GlyrQuery q;
    glyr_query_destroy(NULL);
    glyr_query_destroy(&q);
    glyr_query_init(&q);
    glyr_query_destroy(&q);
}
END_TEST

//--------------------

START_TEST(test_glyr_free_list)
{
    glyr_free_list(NULL);
    GlyrMemCache * new1 = glyr_cache_new();
    GlyrMemCache * new2 = glyr_cache_new();
    GlyrMemCache * new3 = glyr_cache_new();
    new2->next = new3;
    new2->prev = new1;
    glyr_free_list(new2);
}
END_TEST

//--------------------

START_TEST(test_glyr_cache_set_data)
{
    glyr_cache_set_data(NULL,"lala",10);

    GlyrMemCache * c = glyr_cache_new();
    c->data = g_strdup("Hello?");
    c->size = strlen("Hello?");
    glyr_cache_update_md5sum(c);    
    unsigned char old_sum[16] = {0};
    memcpy(old_sum,c->md5sum,16);
    size_t old_size = c->size;

    glyr_cache_set_data(c,g_strdup(" World!"),-1);
    
    fail_if(g_strcmp0(c->data," World!"),NULL);
    fail_if(old_size == c->size,NULL);
    fail_if(!memcmp(old_sum,c->md5sum,16),NULL);

    glyr_cache_free(c);
}
END_TEST

//--------------------

START_TEST(test_glyr_cache_write)
{
    glyr_init();

    glyr_cache_write(NULL,"/tmp");
    GlyrMemCache * tmp = glyr_cache_new();
    glyr_cache_write(tmp,NULL);

    glyr_cache_set_data(tmp,g_strdup("Test data of the cache"),-1);
    glyr_cache_write(tmp,"stdout");
    glyr_cache_write(tmp,"stderr");
    glyr_cache_write(tmp,"null");
    glyr_cache_write(tmp,"/tmp/test.txt");

    glyr_cache_free(tmp);
    glyr_cleanup();
}
END_TEST

//--------------------
//--------------------
//--------------------

START_TEST(test_glyr_download)
{
    glyr_init();
    atexit(glyr_cleanup);
    GlyrMemCache * c = glyr_download("www.google.de",NULL);
    fail_unless(c != NULL,"Could not load www.google.de");
    glyr_cache_free(c);
}
END_TEST

//--------------------

static bool rel_check_single_item(GlyrMemCache * c)
{
    bool result = true;
    if(strlen(c->data) != c->size)
    {
        puts("Missmatch of data's size and cache->size");
        result = false;
    }    

    if(c->type != GLYR_TYPE_RELATION)
    {
        puts("Bad type!");
        result = false;
    }

    unsigned char nullsum[16] = {0};
    if(memcmp(nullsum,c->md5sum,16) == 0)
    {
        puts("Empty checksum!");
        result = false;
    }

    return result;
}

START_TEST(test_glyr_basic_relations)
{

    GlyrQuery q;
    GLYR_ERROR err;
    int length;

    glyr_init();
    atexit(glyr_cleanup);
    
    setup(&q,GLYR_GET_RELATIONS,1);
    glyr_opt_from(&q,"generated");
    glyr_opt_artist(&q,"Metallica");

    GlyrMemCache * list = glyr_get(&q,&err,&length);
    if(err != GLYRE_OK)
    {
        puts(glyr_strerror(err));
    }

    GlyrMemCache * to_free = list;
    while(list != NULL)
    {
        fail_unless(rel_check_single_item(list),"Item check failed");
        list = list->next;
    }

    unsetup(&q,to_free);
}
END_TEST 

//--------------------

static GLYR_ERROR test_callback_ok(GlyrMemCache * c, GlyrQuery * q)
{
    return GLYRE_OK;
}

static GLYR_ERROR test_callback_post(GlyrMemCache * c, GlyrQuery * q)
{
    return GLYRE_STOP_POST;
}

static GLYR_ERROR test_callback_pre(GlyrMemCache * c, GlyrQuery * q)
{
    return GLYRE_STOP_PRE;
}

static GLYR_ERROR test_callback_skip1(GlyrMemCache * c, GlyrQuery * q)
{
    GLYR_ERROR rc = GLYRE_OK;
    int * counter = q->callback.user_pointer;
    if(counter[0]++ % 2 == 0)
    {
       rc = GLYRE_SKIP;
    }
    return rc;
}

static GLYR_ERROR test_callback_skip2(GlyrMemCache * c, GlyrQuery * q)
{
    return GLYRE_SKIP;
}

START_TEST(test_glyr_opt_dlcallback)
{
    glyr_init();
    atexit(glyr_cleanup);

    GlyrQuery q;
    GlyrMemCache * list;
    gint length = -1;
    setup(&q,GLYR_GET_ARTIST_PHOTOS,2);
    glyr_opt_useragent(&q,"I am a useragent.");
    glyr_opt_verbosity(&q,0);

    glyr_opt_dlcallback(&q,test_callback_ok,NULL);
    list = glyr_get(&q,NULL,&length);
    g_print("L = %d\n",length);
    fail_unless(length == 2,NULL);
    glyr_free_list(list);

    glyr_opt_dlcallback(&q,test_callback_post,NULL);
    list = glyr_get(&q,NULL,&length);
    g_print("L = %d\n",length);
    fail_unless(length == 1,NULL);
    glyr_free_list(list);

    glyr_opt_dlcallback(&q,test_callback_pre,NULL);
    list = glyr_get(&q,NULL,&length);
    g_print("L = %d\n",length);
    fail_unless(length == 0,NULL);
    glyr_free_list(list);

    int ctr = 0;
    glyr_opt_dlcallback(&q,test_callback_skip1,&ctr);
    list = glyr_get(&q,NULL,&length);
    g_print("L = %d\n",length);
    fail_unless(length == 1,NULL);
    glyr_free_list(list);

    glyr_opt_dlcallback(&q,test_callback_skip2,NULL);
    list = glyr_get(&q,NULL,&length);
    fail_unless(length == 0,NULL);
    g_print("L = %d\n",length);
    glyr_free_list(list);

    glyr_query_destroy(&q);
}
END_TEST

//--------------------

START_TEST(test_glyr_opt_redirects)
{
    GlyrQuery q;
    setup(&q,GLYR_GET_COVERART,1);

    glyr_opt_from(&q,"amazon");
    glyr_opt_redirects(&q,0);

    int length = 0; 
    GLYR_ERROR err = GLYRE_OK;
    GlyrMemCache * list = glyr_get(&q,&err,&length);
    fail_unless(list == NULL,"should fail due to redirects");
    if(err != GLYRE_OK)
    {
        puts(glyr_strerror(err));
    }

    unsetup(&q,list);
}
END_TEST

//--------------------

START_TEST(test_glyr_opt_number)
{
    glyr_init();
    atexit(glyr_cleanup);

    GlyrQuery q;
    int length = 0;
    setup(&q,GLYR_GET_ARTIST_PHOTOS,4);
    GlyrMemCache * list = glyr_get(&q,NULL,&length);

    fail_unless(length == 4,NULL);
    
    unsetup(&q,list);
}
END_TEST

//--------------------
// from
// plugmax

START_TEST(test_glyr_opt_allowed_formats)
{
    glyr_init();
    atexit(glyr_cleanup);
    
    GlyrQuery q;
    setup(&q,GLYR_GET_COVERART,1);
    glyr_opt_verbosity(&q,0);
    glyr_opt_allowed_formats(&q,"png");
    
    int length = 0;
    GlyrMemCache * list = glyr_get(&q,NULL,&length);

    fail_if(strcmp(list->img_format,"png") != 0,NULL);

    unsetup(&q,list);
}
END_TEST

//--------------------

START_TEST(test_glyr_opt_proxy)
{
    glyr_init();
    atexit(glyr_cleanup);
    
    GlyrQuery q;
    setup(&q,GLYR_GET_COVERART,1);
    glyr_opt_verbosity(&q,0);
    glyr_opt_proxy(&q,"I can haz Existence?");
    
    GlyrMemCache * list = glyr_get(&q,NULL,NULL);
    fail_unless(list == NULL,NULL);

    glyr_cache_free(list);
    glyr_query_destroy(&q);
}
END_TEST

//--------------------

Suite * create_test_suite(void)
{
  Suite *s = suite_create ("Libglyr API");

  /* Core test case */
  TCase * tc_core = tcase_create ("Init");
  tcase_add_test(tc_core, test_glyr_init);
  tcase_add_test(tc_core, test_glyr_destroy_before_init);
  tcase_add_test(tc_core, test_glyr_query_init);
  tcase_add_test(tc_core, test_glyr_query_destroy);
  tcase_add_test(tc_core, test_glyr_free_list);
  tcase_add_test(tc_core, test_glyr_cache_free);
  tcase_add_test(tc_core, test_glyr_cache_copy);
  tcase_add_test(tc_core, test_glyr_cache_set_data);
  tcase_add_test(tc_core, test_glyr_cache_write);
  tcase_add_test(tc_core, test_glyr_download);
  suite_add_tcase(s, tc_core);
  TCase * tc_options = tcase_create ("Options");
  tcase_set_timeout(tc_options,GLYR_DEFAULT_TIMEOUT * 2);
  tcase_add_test(tc_options, test_glyr_opt_dlcallback);
  tcase_add_test(tc_options, test_glyr_opt_redirects);
  tcase_add_test(tc_options, test_glyr_opt_number);
  tcase_add_test(tc_options, test_glyr_opt_allowed_formats);
  tcase_add_test(tc_options, test_glyr_opt_proxy);
  suite_add_tcase(s, tc_options);
  TCase * tc_basic = tcase_create ("Basic");
  tcase_add_test(tc_core, test_glyr_basic_relations);
  suite_add_tcase(s, tc_basic);

  return s;
}

//--------------------

int main(void)
{
    int number_failed;
    Suite *s = create_test_suite();

    SRunner * sr = srunner_create(s);
    srunner_set_log(sr, "check_glyr_api.log");
    srunner_run_all(sr, CK_VERBOSE);

    number_failed = srunner_ntests_failed(sr);
    srunner_free (sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
};
