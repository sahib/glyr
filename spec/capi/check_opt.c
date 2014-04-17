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

//--------------------

static GLYR_ERROR test_callback_ok(GlyrMemCache *c, GlyrQuery *q)
{
    return GLYRE_OK;
}

static GLYR_ERROR test_callback_post(GlyrMemCache *c, GlyrQuery *q)
{
    return GLYRE_STOP_POST;
}

static GLYR_ERROR test_callback_pre(GlyrMemCache *c, GlyrQuery *q)
{
    return GLYRE_STOP_PRE;
}

static GLYR_ERROR test_callback_skip1(GlyrMemCache *c, GlyrQuery *q)
{
    GLYR_ERROR rc = GLYRE_OK;
    int *counter = q->callback.user_pointer;
    if(counter[0]++ % 2 == 0) {
        rc = GLYRE_SKIP;
    }
    return rc;
}

static GLYR_ERROR test_callback_skip2(GlyrMemCache *c, GlyrQuery *q)
{
    return GLYRE_SKIP;
}

START_TEST(test_glyr_opt_dlcallback)
{
    GlyrQuery q;
    GlyrMemCache *list;
    gint length = -1;
    setup(&q, GLYR_GET_ARTIST_PHOTOS, 2);
    glyr_opt_useragent(&q, "I am a useragent.");
    glyr_opt_verbosity(&q, 0);

    glyr_opt_dlcallback(&q, test_callback_ok, NULL);
    list = glyr_get(&q, NULL, &length);
    g_print("L = %d\n", length);
    fail_unless(length == 2, NULL);
    glyr_free_list(list);

    glyr_opt_dlcallback(&q, test_callback_post, NULL);
    list = glyr_get(&q, NULL, &length);
    g_print("L = %d\n", length);
    fail_unless(length == 1, NULL);
    glyr_free_list(list);

    glyr_opt_dlcallback(&q, test_callback_pre, NULL);
    list = glyr_get(&q, NULL, &length);
    g_print("L = %d\n", length);
    fail_unless(length == 0, NULL);
    glyr_free_list(list);

    int ctr = 0;
    glyr_opt_dlcallback(&q, test_callback_skip1, &ctr);
    list = glyr_get(&q, NULL, &length);
    g_print("L = %d\n", length);
    fail_unless(length == 1, NULL);
    glyr_free_list(list);

    glyr_opt_dlcallback(&q, test_callback_skip2, NULL);
    list = glyr_get(&q, NULL, &length);
    fail_unless(length == 0, NULL);
    g_print("L = %d\n", length);
    glyr_free_list(list);

    glyr_query_destroy(&q);
}
END_TEST

//--------------------

START_TEST(test_glyr_opt_redirects)
{
    GlyrQuery q;
    setup(&q, GLYR_GET_COVERART, 1);

    glyr_opt_from(&q, "amazon");
    glyr_opt_redirects(&q, 0);

    int length = 0;
    GLYR_ERROR err = GLYRE_OK;
    GlyrMemCache *list = glyr_get(&q, &err, &length);
    fail_unless(list == NULL, "should fail due to redirects");
    if(err != GLYRE_OK) {
        puts(glyr_strerror(err));
    }

    unsetup(&q, list);
}
END_TEST

//--------------------

START_TEST(test_glyr_opt_number)
{
    GlyrQuery q;
    int length = 0;
    setup(&q, GLYR_GET_ARTIST_PHOTOS, 4);
    GlyrMemCache *list = glyr_get(&q, NULL, &length);

    fail_unless(length == 4, NULL);

    unsetup(&q, list);
}
END_TEST

//--------------------
// from
// plugmax

START_TEST(test_glyr_opt_allowed_formats)
{
    GlyrQuery q;
    setup(&q, GLYR_GET_COVERART, 1);
    glyr_opt_verbosity(&q, 0);
    glyr_opt_allowed_formats(&q, "png");

    int length = 0;
    GlyrMemCache *list = glyr_get(&q, NULL, &length);

    fail_if(strcmp(list->img_format, "png") != 0, NULL);

    unsetup(&q, list);
}
END_TEST

//--------------------

START_TEST(test_glyr_opt_proxy)
{
    GlyrQuery q;
    setup(&q, GLYR_GET_COVERART, 1);
    glyr_opt_verbosity(&q, 0);
    glyr_opt_proxy(&q, "I can haz Existence?");

    GlyrMemCache *list = glyr_get(&q, NULL, NULL);
    fail_unless(list == NULL, NULL);

    glyr_cache_free(list);
    glyr_query_destroy(&q);
}
END_TEST

//--------------------

Suite *create_test_suite(void)
{
    Suite *s = suite_create("Libglyr");

    /* Core test case */
    TCase *tc_options = tcase_create("Options");
    tcase_set_timeout(tc_options, GLYR_DEFAULT_TIMEOUT * 4);
    tcase_add_test(tc_options, test_glyr_opt_dlcallback);
    tcase_add_test(tc_options, test_glyr_opt_redirects);
    tcase_add_test(tc_options, test_glyr_opt_number);
    tcase_add_test(tc_options, test_glyr_opt_allowed_formats);
    tcase_add_test(tc_options, test_glyr_opt_proxy);
    suite_add_tcase(s, tc_options);
    return s;
}

//--------------------

int main(void)
{
    init();

    int number_failed;
    Suite *s = create_test_suite();

    SRunner *sr = srunner_create(s);
    srunner_set_log(sr, "check_glyr_opt.log");
    srunner_run_all(sr, CK_VERBOSE);

    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
};
