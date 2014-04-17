/***********************************************************
 * This file is part of glyr
 * + a command-line tool and library to download various sort of musicrelated metadata.
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

#include "test_common.h"
#include <check.h>
#include <glib.h>

//--------------------
//--------------------

START_TEST(test_glyr_init)
{
    GlyrQuery q;
    fail_unless(glyr_get(NULL, NULL, NULL) == NULL, "glyr_get() should return NULL without INIT");
    fail_unless(glyr_get(&q, NULL, NULL) == NULL, "glyr_get() should still return NULL, even with uninitalized query.");

    glyr_init();
    fail_unless(glyr_get(NULL, NULL, NULL) == NULL, "glyr_get() should return NULL without INIT");
    fail_unless(glyr_get(&q, NULL, NULL) == NULL, "should not access bad memory");

    glyr_init();
    fail_unless(glyr_get(NULL, NULL, NULL) == NULL, "glyr_get() should return NULL without INIT");
    fail_unless(glyr_get(&q, NULL, NULL) == NULL, "should not access bad memory");

    glyr_cleanup();
    glyr_cleanup();
    fail_unless(glyr_get(NULL, NULL, NULL) == NULL, "glyr_get() should return NULL without INIT");
    fail_unless(glyr_get(&q, NULL, NULL) == NULL, "should not access bad memory");

    glyr_query_init(&q);
    fail_unless(q.is_initalized == 0xDEADBEEF, NULL);
    fail_unless(glyr_get(&q, NULL, NULL) == NULL, "should not access bad memory");
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
    fail_unless(q.is_initalized == 0xDEADBEEF, NULL);

    GlyrQuery *alloc = malloc(sizeof(GlyrQuery));
    glyr_query_init(alloc);
    fail_unless(alloc->is_initalized == 0xDEADBEEF, NULL);

    glyr_query_destroy(&q);
}
END_TEST

//--------------------

START_TEST(test_glyr_cache_free)
{
    GlyrMemCache *test = glyr_cache_new();
    glyr_cache_free(NULL);
    glyr_cache_free(test);
}
END_TEST

//--------------------

START_TEST(test_glyr_cache_copy)
{
    GlyrMemCache *test = glyr_cache_new();
    glyr_cache_set_data(test, g_strdup("some data"), -1);
    test->next = (GlyrMemCache *) 0x1010;
    test->prev = (GlyrMemCache *) 0x0101;

    GlyrMemCache *copy = glyr_cache_copy(test);
    fail_unless(memcmp(copy->data, test->data, test->size) == 0, "Should have the same data");
    fail_unless(copy->next == NULL, NULL);
    fail_unless(copy->prev == NULL, NULL);

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
    GlyrMemCache *new1 = glyr_cache_new();
    GlyrMemCache *new2 = glyr_cache_new();
    GlyrMemCache *new3 = glyr_cache_new();
    new2->next = new3;
    new2->prev = new1;
    glyr_free_list(new2);
}
END_TEST

//--------------------

START_TEST(test_glyr_cache_set_data)
{
    glyr_cache_set_data(NULL, "lala", 10);

    GlyrMemCache *c = glyr_cache_new();
    c->data = g_strdup("Hello?");
    c->size = strlen("Hello?");
    glyr_cache_update_md5sum(c);
    unsigned char old_sum[16] = {0};
    memcpy(old_sum, c->md5sum, 16);
    size_t old_size = c->size;

    glyr_cache_set_data(c, g_strdup(" World!"), -1);

    fail_if(g_strcmp0(c->data, " World!"), NULL);
    fail_if(old_size == c->size, NULL);
    fail_if(!memcmp(old_sum, c->md5sum, 16), NULL);

    glyr_cache_free(c);
}
END_TEST

//--------------------

START_TEST(test_glyr_cache_write)
{
    glyr_init();

    glyr_cache_write(NULL, "/tmp");
    GlyrMemCache *tmp = glyr_cache_new();
    glyr_cache_write(tmp, NULL);

    glyr_cache_set_data(tmp, g_strdup("Test data of the cache"), -1);
    glyr_cache_write(tmp, "stdout");
    glyr_cache_write(tmp, "stderr");
    glyr_cache_write(tmp, "null");
    glyr_cache_write(tmp, "/tmp/test.txt");

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
    GlyrMemCache *c = glyr_download("www.duckduckgo.com", NULL);
    fail_unless(c != NULL, "Could not load www.google.de");
    glyr_cache_free(c);
}
END_TEST

//--------------------

Suite *create_test_suite(void)
{
    Suite *s = suite_create("Libglyr API");

    /* Core test case */
    TCase *tc_core = tcase_create("Init");
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
    return s;
}

//--------------------

int main(void)
{
    init();

    int number_failed;
    Suite *s = create_test_suite();

    SRunner *sr = srunner_create(s);
    srunner_set_log(sr, "check_glyr_api.log");
    srunner_run_all(sr, CK_VERBOSE);

    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
};
