#include <locale.h>
#include <stdio.h>

#include "../testing_utils.h"
#include "hashmap.h"
#include <stdlib.h>
#include <string.h>

#include "../carray/carray_types.h"

// -----------------------------------------------
// setup and teardown fixtures
// ----------------------------------------------
// create a HashMap for use in test cases
void *  hashmap_fixture(const MunitParameter params[], void* user_data) {
    HashMap *map = map_create(10, free);
    munit_assert_not_null(map);
    return map;
}

// to free the hashmap created by the hashmap_fixture after a test
void hashmap_free(void * fixture) {
    free(fixture);
}

// -------------------------------------------------
// test cases
// -------------------------------------------------

MunitResult test_create_and_free(const MunitParameter params[], void* fixture) {
    HashMap *map = map_create(10, nullptr);
    munit_assert_ptr_not_null(map);
    map_free(map);
    return MUNIT_OK;
}

MunitResult test_put_and_get_int(const MunitParameter params[], void* fixture) {
    HashMap *map = fixture;

    map_put(map, 1, 42);
    MapValue retrieved = map_get(map, 1);
    munit_assert_int(retrieved.vlong, ==, 42);

    int foo = 2;
    int bar = 67;
    map_put(map, foo, bar);
    retrieved = map_get(map, foo);
    munit_assert_int(retrieved.vlong, ==, bar);

    return MUNIT_OK;
}

MunitResult test_put_and_get_string(const MunitParameter params[], void* fixture) {
    HashMap *map = fixture;
    map_put(map, 1, "hello");
    MapValue retrieved = map_get(map, 1);
    munit_assert_string_equal(retrieved.vstring, "hello");
    return MUNIT_OK;
}

MunitResult test_update_value(const MunitParameter params[], void* fixture) {
    HashMap *map = fixture;
    map_put(map, 1, "hello");
    map_put(map, 1, "world"); // This should free val1 and replace it with val2
    MapValue retrieved = map_get(map, 1);
    munit_assert_string_equal(retrieved.vstring, "world");
    return MUNIT_OK;
}

MunitResult test_remove_key(const MunitParameter params[], void* fixture) {
    HashMap *map = fixture;
    map_put(map, 1, "hello");
    map_remove(map, 1);
    MapValue retrieved = map_get(map, 1);
    munit_assert_int(retrieved.value_type, ==, MAP_TYPE_NULL);
    munit_assert_ptr_null(retrieved.vvoid_ptr);
    return MUNIT_OK;
}

MunitResult test_put_and_get_str_key(const MunitParameter params[], void* fixture) {
    HashMap *map = fixture;

    map_put(map, "hello", "world");
    map_put(map, "good", "morning");
    MapValue got_char = map_get(map, "hello");
    munit_assert_string_equal("world", got_char.vstring);

    map_remove(map, "good");
    MapValue retrieved = map_get(map, "good");
    munit_assert_ptr_null(retrieved.vstring);

    return MUNIT_OK;
}

MunitResult test_put_and_get_bool_values(const MunitParameter params[], void* fixture) {
    HashMap *map = fixture;
    bool b1 = false;
    map_put(map, "false", false);
    map_put(map, "true", !b1);

    MapValue result1 = map_get(map, "false");
    MapValue result2 = map_get(map, "true");

    munit_assert_true(result2.vlong);
    munit_assert_false(result1.vlong);

    return MUNIT_OK;
}

MunitResult test_klong_vstring(const MunitParameter params[], void* fixture) {
    HashMap *map = fixture;
    for (int i = 0; i < 100; ++i) {
        char search_string[10]; // max 4 chars for value of i, plus 5 for 'hello', plus terminator
        snprintf(search_string, 10, "hello%d",i+1);
        map_put(map, i, search_string );
    }
    for (int i=0; i< 100; ++i) {
        char search_string[10]; // max 4 chars for value of i, plus 5 for 'hello', plus terminator
        snprintf(search_string,10, "hello%d",i+1);

        MapValue value = map_get(map, i );

        munit_assert_string_equal( search_string, value.vstring);
    }

    return MUNIT_OK;
}

MunitResult test_generic_put(const MunitParameter params[], void* fixture) {
    HashMap *map = fixture;
    map_put(map, 42, "foo");
    map_put(map, "bar", 123);
    map_put(map, (short)67, "short!");
    map_put(map, (float)67.767, "float!");

    MapValue retrieved_str = map_get(map, 42);
    munit_assert_string_equal(retrieved_str.vstring, "foo");

    MapValue retrieved_long = map_get(map, "bar");
    munit_assert_long(retrieved_long.vlong, ==, 123);

    return MUNIT_OK;
}

MunitResult test_clear(const MunitParameter params[], void* fixture) {
    HashMap *map = fixture;

    map_put(map, 1, "dog");
    map_put(map, 2, "cat");
    munit_assert_int(2, ==,  map->size);
    map_clear( map );
    munit_assert_int(0, ==,  map->size);
    munit_assert_true(map_is_empty(map));
    return MUNIT_OK;
}

MunitResult test_contains_key(const MunitParameter params[], void* fixture) {
    HashMap *map = fixture;

    map_put(map, 1, "dog");
    map_put(map, 2.0, "cat");
    map_put(map, "wolf", "big bad");
    int int1 = 42;
    void * vptr = (void *)&int1;
    map_put(map, vptr, "void pointer");

    munit_assert_true(map_contains_key(map, 1));
    munit_assert_true(map_contains_key(map, 2.0));
    munit_assert_true(map_contains_key(map, "wolf"));
    munit_assert_true(map_contains_key(map, vptr));

    return MUNIT_OK;

}


MunitResult test_10K_inserts(const MunitParameter params[], void* fixture) ;
MunitResult test_10K_string_inserts(const MunitParameter params[], void* fixture) ;


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
// clang -std=c23 -o ./out/test_hashmap.out test_hashmap.c hashmap.c ../munit/munit.c
int main(int argc, char *argv[argc + 1]) {
    setlocale(LC_NUMERIC, "en_US.UTF-8");   // use user's system locale

    MunitTest tests[] = {
        munit_test(test_create_and_free),
        munit_test("/test_put_and_get_int", test_put_and_get_int),
        munit_test(test_put_and_get_string),
        munit_test(test_update_value),
        munit_test(test_remove_key),
        munit_test(test_put_and_get_str_key),
        munit_test(test_put_and_get_bool_values),
        munit_test(test_klong_vstring),
        munit_test(test_generic_put),
        munit_test(test_clear),
        munit_test(test_contains_key),

        MUNIT_NULL_TEST,
    };

    apply_fixture(tests, hashmap_fixture, hashmap_free);


     MunitSuite suite = {
        "/hashmap", /* name */
        tests, /* tests */
        nullptr, /* suites */
        1, /* iterations */
        MUNIT_SUITE_OPTION_NONE /* options */
      };


    int result = 0;
    result = munit_suite_main(&suite, nullptr, argc, argv);

    test_10K_inserts(nullptr, nullptr);
    test_10K_string_inserts(nullptr, nullptr);
    return result;

}


// -------------------------
// ad hoc tests
// -------------------------
void test_repr(void) {
    HashMap *map = map_create(0, free);
    map_repr_HashMap(map, true); print("");
    map_free(map);

    map = map_create(10, free);
    map_repr_HashMap(map, true); print("");
    map_free(map);

    map = map_create(16, free);
    map_repr_HashMap(map, true); print("");
    map_free(map);

    map = map_create(17, free);

    map_free(map);
}

MunitResult test_10K_inserts(const MunitParameter params[], void* fixture) {
    print("test_10K_inserts");
    constexpr size_t N = 10'0;
    HashMap *map = map_create(0, free);
    size_t buffer_size = 20;  // max 9 chars for value of i, plus 5 for 'hello', plus terminator
    for (int i = 0; i < N; ++i) {
        char search_string[buffer_size] = {};
        snprintf(search_string, buffer_size, "hello%d",i+1);
        map_put(map, i, search_string );
    }
    for (int i=0; i< N; ++i) {
        char search_string[buffer_size] = {}; // max 4 chars for value of i, plus 5 for 'hello', plus terminator
        snprintf(search_string,buffer_size, "hello%d",i+1);
        MapValue value = map_get(map, i );
        // print("search string = %s, value = %s", search_string, value);

        munit_assert_string_equal( search_string, value.vstring);
    }

    print("");

    // printf("finished adding %zu items to map.\n", N);
    // map_repr_HashMap(map, false);


    fflush(stdout);
    // printf("about to call map_free:\n");
    fflush(stdout);

    map_free(map);

    return MUNIT_OK;
}


MunitResult test_10K_string_inserts(const MunitParameter params[], void* fixture) {
    print("test_10K_string_inserts");
    constexpr size_t N = 10;
    HashMap *map = map_create(0, free);

    char * value_str = "hello world";
    for (int i = 0; i < N; ++i) {
        map_put(map, i, value_str );
    }
    for (int i=0; i< N; ++i) {
        MapValue value = map_get(map, i );
        munit_assert_string_equal( value_str, value.vstring);
    }

    // print("");

    // printf("finished adding %zu items to map.\n", N);
    // map_repr_HashMap(map, false);

    MapValue count = map_get(map->intern_strings, "hello world");
    // printf("'hello reference world' count = %ld\n", count.vlong);
    // map_repr_HashMap(map->intern_strings, false);

    print("\n");

    //try removing keys, the reference count should reduce by 1 each time
    for (int i=0; i< N; ++i) {
        // print("\nremoving #%d", i);
        map_remove(map, i );
        munit_assert_int( N-i - 1, ==, map->size);
        count = map_get(map->intern_strings, "hello world");
        // printf("   count MapValue value_type: %d\n", count.value_type);
        // printf("'hello reference world' count = %ld\n", count.vlong);
        // map_repr_HashMap(map->intern_strings, false);
        munit_assert_int( N-i - 1, ==, count.vlong);
    }

    fflush(stdout);
    // printf("about to call map_free:\n");
    fflush(stdout);
    map_free(map);

    return MUNIT_OK;
}