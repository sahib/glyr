#include "test_common.h"

#include "../../lib/cache.h"

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
    system("rm -rf /tmp/check/metadata.db");
}

//--------------------
//--------------------
//--------------------
//--------------------

START_TEST(test_create_db)
{
    init();

    glyr_db_init(NULL);
    GlyrDatabase * db = glyr_db_init("/tmp");
    glyr_db_destroy(db);
    glyr_db_destroy(NULL);
}
END_TEST

//--------------------

START_TEST(test_simple_db)
{
    init();
    GlyrQuery q;
    setup(&q,GLYR_GET_LYRICS,10);
    glyr_opt_artist(&q,"Equi");
    glyr_opt_title(&q,"lala");

    cleanup_db();

    GlyrDatabase * db = glyr_db_init("/tmp/check");
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

/* Write artist|album|title, select only artist|title */
START_TEST(test_intelligent_lookup)
{
    init();

    GlyrQuery alt;
    glyr_query_init(&alt);

    gchar * artist = "Equilibrium";
    gchar * album  = "Sagas";
    gchar * title  = "Wurzelbert";

    glyr_opt_artist(&alt,artist);
    glyr_opt_album (&alt,album );
    glyr_opt_title (&alt,title );
    glyr_opt_type  (&alt,GLYR_GET_LYRICS);

    GlyrMemCache * subject = glyr_cache_new();
    glyr_cache_set_data(subject,g_strdup("These are lyrics. Really."),-1);

    cleanup_db();
    GlyrDatabase * db = glyr_db_init("/tmp/check");
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
    cleanup_db();
    init();

    GlyrDatabase * db = glyr_db_init("/tmp/check");
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
  tcase_add_test(tc_dbcache, test_intelligent_lookup);
  tcase_add_test(tc_dbcache, test_db_editplace);
  suite_add_tcase(s, tc_dbcache);
  return s;
}

//--------------------

int main(void)
{
    int number_failed;
    Suite * s = create_test_suite();

    SRunner * sr = srunner_create(s);
    srunner_set_log(sr, "check_glyr_opt.log");
    srunner_run_all(sr, CK_VERBOSE);

    number_failed = srunner_ntests_failed(sr);
    srunner_free (sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
};
