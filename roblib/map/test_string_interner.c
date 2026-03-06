// test_string_interner.c
//
// Copyright (c) Rob Ross 2026. 
//
//
// Created 2026/03/03 22:32:05 PST

#include <locale.h>
#include <stdio.h>

#include "string_interner.h"
#include "../testing_utils.h"
#include "hashmap.h"

struct InternStringMap {
    HashMap *map;
};

// -----------------------------------------------
// setup and teardown fixtures
// ----------------------------------------------
// create a HashMap for use in test cases
void *  intstr_fixture(const MunitParameter params[], void* user_data) {
    InternStringMap *ismap = instr_create(16);
    munit_assert_not_null(ismap);
    HashMap *map = ismap->map;
    munit_assert_not_null(map);
    return ismap;
}

// to free the hashmap created by the intstr_fixture after a test
void hashintstr_free(void * fixture) {
    instr_destroy((InternStringMap*)fixture);
}

// -------------------------------------------------
// test cases
// -------------------------------------------------

MunitResult test_create_and_free(const MunitParameter params[], void* fixture) {
    InternStringMap *ismap = instr_create(16);
    HashMap *map = ismap->map;
    munit_assert_ptr_not_null(map);
    instr_destroy(ismap);
    return MUNIT_OK;
}


MunitResult test_put_and_get_string(const MunitParameter params[], void* fixture) {
    InternStringMap *map = fixture;
    instr_put(map, "hello",1);
    long vlong = instr_get_count(map, "hello");
    munit_assert_int(1, ==, vlong);
    instr_put(map, "hello", 22);
    instr_put(map, "hello", 3);
    vlong = instr_get_count(map, "hello");
    munit_assert_int(3, ==, vlong);
    return MUNIT_OK;
}

MunitResult test_refcount(const MunitParameter params[], void* fixture) {
    InternStringMap *map = fixture;
    instr_ref(map, "hello");
    instr_ref(map, "hello");
    long vlong = instr_get_count(map, "hello");
    munit_assert_int(2, ==, vlong);
    MapValue mv = map_get(map->map, "hello");
    munit_assert_int(2, ==, mv.vlong);
    return MUNIT_OK;
}

MunitResult test_remove_key(const MunitParameter params[], void* fixture) {
    InternStringMap *map = fixture;
    instr_put(map, "hello",1);
    instr_remove(map, "hello");
    long vlong = instr_get_count(map, "hello");
    munit_assert_int(0, ==, vlong);
    return MUNIT_OK;
}

MunitResult test_put_and_get_str_key(const MunitParameter params[], void* fixture) {
    InternStringMap *map = fixture;

    instr_put(map, "hello",1);
    instr_put(map, "good", 1);
    long vlong= instr_get_count(map, "hello");
    munit_assert_int(1, ==, vlong);

    instr_remove(map, "good");
    vlong = instr_get_count(map, "good");
    munit_assert_int(0, ==, vlong);

    return MUNIT_OK;
}


MunitResult test_100_string_different(const MunitParameter params[], void* fixture) {
    InternStringMap *map = fixture;
    for (int i = 0; i < 100; ++i) {
        char strkey[10]; // max 4 chars for value of i, plus 5 for 'hello', plus terminator
        snprintf(strkey, 10, "hello%d",i+1);
        instr_put(map, strkey, i );
    }
    munit_assert_int(100, ==, instr_size(map));

    for (int i=0; i< 100; ++i) {
        char search_string[10]; // max 4 chars for value of i, plus 5 for 'hello', plus terminator
        snprintf(search_string,10, "hello%d",i+1);

        long vlong = instr_get_count(map, search_string );

        munit_assert_int( 1, ==, vlong);
    }


    return MUNIT_OK;
}


MunitResult test_clear(const MunitParameter params[], void* fixture) {
    InternStringMap *map = fixture;

    instr_put(map,"dog", 1);
    instr_put(map, "cat", 1);
    munit_assert_int(2, ==,  map->map->size);
    instr_clear( map );
    munit_assert_int(0, ==,  map->map->size);
    munit_assert_true(instr_is_empty(map));
    return MUNIT_OK;
}

MunitResult test_contains_key(const MunitParameter params[], void* fixture) {
    InternStringMap *map = fixture;

    instr_put(map,"dog",1);
    instr_put(map, "cat", 1);
    instr_put(map, "wolf",1 );

    munit_assert_true(instr_contains_key(map, "dog"));
    munit_assert_true(instr_contains_key(map, "cat"));
    munit_assert_true(instr_contains_key(map, "wolf"));
    munit_assert_false(instr_contains_key(map, "no such key"));

    return MUNIT_OK;

}




// ------------------------------------
// end test cases
// ------------------------------------


void apply_fixture(MunitTest tests[static 1], MunitTestSetup setup, MunitTestTearDown tear_down) {
    size_t test_index = 0;
    do {
        MunitTest *test = &tests[test_index++];
        // ReSharper disable once CppIncompatiblePointerConversion
        test->setup = setup;
        test->tear_down = tear_down;
    } while ( tests[test_index].name != nullptr );


}
// make
// clang -std=c23 -fsanitize=address -fsanitize=leak -Wall -Werror -o ./out/test_string_interner.out test_string_interner.c string_interner.c hashmap.c ../munit/munit.c
int main(int argc, char *argv[argc + 1]) {
    setlocale(LC_NUMERIC, "en_US.UTF-8");   // use user's system locale

    MunitTest tests[] = {
        munit_test(test_create_and_free),
        munit_test(test_put_and_get_string),
        munit_test(test_refcount),
        munit_test(test_remove_key),
        munit_test(test_put_and_get_str_key),
        munit_test(test_clear),
        munit_test(test_contains_key),

        MUNIT_NULL_TEST,
    };

    apply_fixture(tests, intstr_fixture, hashintstr_free);


     MunitSuite suite = {
        "/instr", /* name */
        tests, /* tests */
        nullptr, /* suites */
        1, /* iterations */
        MUNIT_SUITE_OPTION_NONE /* options */
      };


    int result = 0;
    result = munit_suite_main(&suite, nullptr, argc, argv);
    return result;

}
