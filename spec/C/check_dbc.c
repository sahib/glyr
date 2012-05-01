/***********************************************************
 * This file is part of glyr
 * + a commnandline tool and library to download various sort of musicrelated metadata.
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

#include "../../lib/cache.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

//--------------------

static int counter_callback(GlyrQuery * q, GlyrMemCache * c, void * userptr)
{
    int * i = userptr;
    *i += 1;
    return 0;
}

static int count_db_items(GlyrDatabase * db)
{
    int c = 0;
    glyr_db_foreach(db,counter_callback,&c);
    return c;
}

static void cleanup_db(void)
{
    system("rm -rf /tmp/check/");
}


static GlyrDatabase * setup_db(void)
{
    cleanup_db();
    system("mkdir -p /tmp/check");
    return glyr_db_init("/tmp/check");
}

//--------------------
//--------------------
//--------------------
//--------------------

START_TEST(test_create_db)
{
    glyr_db_init(NULL);
    GlyrDatabase * db = glyr_db_init("/tmp/");
    glyr_db_destroy(db);
    glyr_db_destroy(NULL);

    system("rm /tmp/metadata.db");
}
END_TEST

//--------------------

START_TEST(test_simple_db)
{
    GlyrQuery q;
    setup(&q,GLYR_GET_LYRICS,10);
    glyr_opt_artist(&q,"Equi");
    glyr_opt_title(&q,"lala");

    GlyrDatabase * db = setup_db(); 

    GlyrMemCache * ct = glyr_cache_new();

    glyr_cache_set_data(ct,g_strdup("test"),-1);

    glyr_db_insert(NULL,&q,(GlyrMemCache*)0x1);
    fail_unless(count_db_items(db) == 0, NULL);
    glyr_db_insert(db,NULL,(GlyrMemCache*)0x1);
    fail_unless(count_db_items(db) == 0, NULL);
    glyr_db_insert(db,&q,NULL);
    fail_unless(count_db_items(db) == 0, NULL);
    
    glyr_db_insert(db,&q,ct);
    GlyrMemCache * c = glyr_db_lookup(db,&q);
    fail_unless(c != NULL, NULL);
    fail_unless(c->data != NULL, NULL);
    fail_unless(count_db_items(db) == 1, NULL);

    glyr_db_destroy(db);
    glyr_cache_free(ct);
    glyr_query_destroy(&q);
}
END_TEST

//--------------------

START_TEST(test_iter_db)
{
    GlyrQuery q;
    setup(&q,GLYR_GET_LYRICS,10);
    glyr_opt_artist(&q,"Equi");
    glyr_opt_title(&q,"lala");

    GlyrDatabase * db = setup_db(); 

    GlyrQuery nugget;
    setup(&nugget,GLYR_GET_COVERART,40);
    glyr_opt_artist(&nugget,"A very special artist");
    glyr_opt_album(&nugget,"A very special album");

    GTimer * insert_time = g_timer_new();

    const int N = 5000;
    for(int i = 0; i < N; i++)
    {
        GlyrMemCache * ct = glyr_cache_new();
        glyr_cache_set_data(ct,g_strdup_printf("test# %d",i+1),-1);
        ct->dsrc = g_strdup_printf("Dummy url %d",i+1);

        if(i % 2)
            ct->rating = N ;
        else
            ct->rating = N ;

        if(i % 23)
            glyr_db_insert(db,&q,ct);
        else
            glyr_db_insert(db,&nugget,ct);

        glyr_cache_free(ct);
    }

    g_timer_stop(insert_time);
    g_message("Used %.5f seconds to insert..",g_timer_elapsed(insert_time,NULL));

    /* Check if N items are in DB */
    int cdb = count_db_items(db);
    g_message("Counted %d items",cdb);
    fail_unless(cdb == N, NULL);

    /* Test if case-insensitivity works */
    glyr_opt_artist(&q,"eQuI");
    glyr_opt_title(&q,"LALA");

    float fine_grained = 0.0;
    GTimer * grain_time = g_timer_new();
    
    g_timer_start(insert_time);

    GlyrMemCache * c, * ptr;
    for(int i = 1; i <= N/10; i++)
    {
        g_timer_start(grain_time);
        /* Get a list of the caches */
        if(i % 10)
            c = glyr_db_lookup(db,&q);
        else
            c = glyr_db_lookup(db,&nugget);

        g_timer_stop(grain_time);
        fine_grained += g_timer_elapsed(grain_time,NULL);

        ptr = c;
        fail_if(ptr == NULL);

        int last_rating = INT_MAX;
        int ctr = 0;
        while(ptr) {
            ctr++;
            fail_unless(last_rating >= ptr->rating);
            last_rating = ptr->rating;
            ptr = ptr->next;
        }
        glyr_free_list(c);

        /* Test if we got exactly 10 or 42 items, (honoring number setting) */
 
        if(i % 10)
            fail_unless(ctr == 10);
        else
            fail_unless(ctr == 40);
    }


    g_timer_stop(insert_time);
    g_message("Used %.5f seconds to lookup..",g_timer_elapsed(insert_time,NULL));
    g_message("Used %.5f for actual lookup..",fine_grained);

    glyr_db_destroy(db);
    glyr_query_destroy(&q);
    glyr_query_destroy(&nugget);

    g_timer_destroy(insert_time);
    g_timer_destroy(grain_time);
}
END_TEST 

//--------------------

START_TEST(test_sorted_rating)
{
    const int N = 10;
    
    GlyrDatabase * db = setup_db(); 

    GlyrQuery q;
    glyr_query_init(&q);
    setup(&q,GLYR_GET_LYRICS,N);

    for(int i = 0; i < N; ++i)
    {
        int rate = (i / 2) + 1;

        GlyrMemCache * ct = glyr_cache_new();
        fail_if(ct == NULL);

        glyr_cache_set_data(ct,g_strdup_printf("MyLyrics %d",i),-1);
        ct->dsrc = g_strdup("http://MyLyrics.com");
        ct->rating = rate;
        glyr_db_insert(db,&q,ct);

        glyr_cache_free(ct);
    }

    fail_unless(count_db_items(db) == N);

    GlyrMemCache * list = glyr_db_lookup(db,&q);
    GlyrMemCache * iter = list;
    fail_if(list == NULL);

    double last_timestamp = DBL_MAX;
    int last_rating = INT_MAX;
    while(iter)
    {
        glyr_cache_print(iter);
        fail_unless(last_rating >= iter->rating);
        if(last_rating == iter->rating)
            fail_unless(last_timestamp >= iter->timestamp);

        last_timestamp = iter->timestamp;
        last_rating = iter->rating;
        iter = iter->next;    
    }
    
    glyr_free_list(list);
}
END_TEST

//--------------------

/* Write artist|album|title, select only artist|title */
START_TEST(test_intelligent_lookup)
{
    GlyrDatabase * db = setup_db(); 

    GlyrQuery alt;
    glyr_query_init(&alt);

    gchar * artist = "Аркона";
    gchar * album  = "Ot Serdca k Nebu";
    gchar * title  = "Pokrovy Nebesnogo Startsa (Shrouds Of Celestial Sage)";

    glyr_opt_artist(&alt,artist);
    glyr_opt_album (&alt,album );
    glyr_opt_title (&alt,title );
    glyr_opt_type  (&alt,GLYR_GET_LYRICS);

    GlyrMemCache * subject = glyr_cache_new();
    glyr_cache_set_data(subject,g_strdup("These are lyrics. Really."),-1);

    glyr_db_insert(db,&alt,subject);

    GlyrMemCache * one = glyr_db_lookup(db,&alt);
    fail_if(one == NULL,NULL);
    fail_if(memcmp(one->md5sum,subject->md5sum,16) != 0, NULL);
    glyr_cache_free(one);

    alt.album = NULL;
    GlyrMemCache * two = glyr_db_lookup(db,&alt);
    fail_if(two == NULL,NULL);
    fail_if(memcmp(two->md5sum,subject->md5sum,16) != 0, NULL);
    glyr_cache_free(two);

    fail_unless(count_db_items(db) == 1,NULL); 
    int deleted = glyr_db_delete(db,&alt);
    fail_unless(deleted == 1,NULL);
    fail_unless(count_db_items(db) == 0,NULL); 

    glyr_query_destroy(&alt);
    glyr_db_destroy(db);
}
END_TEST

//--------------------

START_TEST(test_db_editplace)
{

    GlyrDatabase * db = setup_db(); 
    if(db != NULL)
    {
        fail_unless(count_db_items(db) == 0, NULL);

        GlyrMemCache * test_data = glyr_cache_new();
        glyr_cache_set_data(test_data,g_strdup("my test data"),-1);
        GlyrQuery q;
        setup(&q,GLYR_GET_LYRICS,1);
        glyr_db_insert(db,&q,test_data);

        fail_unless(count_db_items(db) == 1, NULL);

        GlyrMemCache * edit_one = glyr_cache_new();
        glyr_cache_set_data(edit_one,g_strdup("my new data"),-1);
        glyr_db_edit(db,&q,edit_one);

        fail_unless(count_db_items(db) == 1, NULL);
        GlyrMemCache * lookup_one = glyr_db_lookup(db,&q);
        fail_unless(memcmp(lookup_one->data,edit_one->data,edit_one->size) == 0, NULL);

        GlyrMemCache * edit_two = glyr_cache_new();
        glyr_cache_set_data(edit_two,g_strdup("my even new data"),-1);
        glyr_db_replace(db,edit_one->md5sum,&q,edit_two);

        fail_unless(count_db_items(db) == 1, NULL);
        GlyrMemCache * lookup_two = glyr_db_lookup(db,&q);
        fail_unless(memcmp(lookup_two->data,edit_two->data,edit_two->size) == 0, NULL);

        glyr_cache_free(lookup_one);
        glyr_cache_free(lookup_two);
        glyr_cache_free(edit_one);
        glyr_cache_free(edit_two);
        glyr_cache_free(test_data);
        glyr_db_destroy(db);
    }
}
END_TEST


//--------------------

Suite * create_test_suite(void)
{
    Suite *s = suite_create ("Libglyr");

    /* Core test case */
    TCase * tc_dbcache = tcase_create("DBCache");
    tcase_set_timeout(tc_dbcache,GLYR_DEFAULT_TIMEOUT * 4);
    tcase_add_test(tc_dbcache, test_create_db);
    tcase_add_test(tc_dbcache, test_simple_db);
    tcase_add_test(tc_dbcache, test_iter_db);
    tcase_add_test(tc_dbcache, test_sorted_rating);
    tcase_add_test(tc_dbcache, test_intelligent_lookup);
    tcase_add_test(tc_dbcache, test_db_editplace);
    suite_add_tcase(s, tc_dbcache);
    return s;
}

//--------------------

int main(void)
{
    init();

    int number_failed;
    Suite * s = create_test_suite();

    SRunner * sr = srunner_create(s);
    srunner_set_log(sr, "check_glyr_opt.log");
    srunner_run_all(sr, CK_VERBOSE);

    number_failed = srunner_ntests_failed(sr);
    srunner_free (sr);

    cleanup_db();
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
};
