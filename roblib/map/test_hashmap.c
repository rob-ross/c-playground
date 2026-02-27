#include <stdio.h>

#include "../testing_utils.h"
#include "hashmap.h"
#include <stdlib.h>
#include <string.h>


MunitResult test_create_and_free(const MunitParameter params[], void* fixture) {
    HashMap *map = create_map(10, nullptr);
    munit_assert_ptr_not_null(map);
    free_map(map);
    return MUNIT_OK;
}

MunitResult test_put_and_get_int(const MunitParameter params[], void* fixture) {
    HashMap *map = create_map(10, free);

    map_put_klong_vlong(map, 1, 42);
    int *retrieved = map_get_klong_vlong(map, 1);
    munit_assert_int(*retrieved, ==, 42);
    free_map(map);
    return MUNIT_OK;
}

MunitResult test_put_and_get_string(const MunitParameter params[], void* fixture) {
    HashMap *map = create_map(10, free);
    map_put_klong_vstring(map, 1, "hello");
    char *retrieved = map_get_klong_vstring(map, 1);
    munit_assert_string_equal(retrieved, "hello");
    free_map(map);
    return MUNIT_OK;
}

MunitResult test_update_value(const MunitParameter params[], void* fixture) {
    HashMap *map = create_map(10, free);
    map_put_klong_vstring(map, 1, "hello");
    map_put_klong_vstring(map, 1, "world"); // This should free val1 and replace it with val2
    char *retrieved = map_get_klong_vstring(map, 1);
    munit_assert_string_equal(retrieved, "world");
    free_map(map);
    return MUNIT_OK;
}

MunitResult test_delete_key(const MunitParameter params[], void* fixture) {
    HashMap *map = create_map(10, free);
    map_put_klong_vstring(map, 1, "hello");
    map_delete_klong(map, 1);
    void *retrieved = map_get_klong_vstring(map, 1);
    munit_assert_ptr_null(retrieved);
    free_map(map);
    return MUNIT_OK;
}

MunitResult test_put_and_get_str_key(const MunitParameter params[], void* fixture) {
    HashMap *map = create_map(10, free);

    map_put_kstring_vstring(map, "hello", "world");
    map_put_kstring_vstring(map, "good", "morning");
    const char *got_char = map_get_kstring_vstring(map, "hello");
    munit_assert_string_equal("world", got_char);

    map_delete_kstring(map, "good");
    const char *retrieved = map_get_kstring_vstring(map, "good");
    munit_assert_ptr_null(retrieved);

    free_map(map);
    return MUNIT_OK;
}

MunitResult test_put_and_get_bool_values(const MunitParameter params[], void* fixture) {
    HashMap *map = create_map(10, free);
    bool b1 = false;
    map_put_kstring_vlong(map, "false", false);
    map_put_kstring_vlong(map, "true", !b1);

    bool result1 = *map_get_kstring_vlong(map, "false");
    bool result2 = *map_get_kstring_vlong(map, "true");

    munit_assert_true(result2);
    munit_assert_false(result1);

    free_map(map);
    return MUNIT_OK;
}

// make
// clang -std=c23 -o ./out/test_hashmap.out test_hashmap.c hashmap.c ../munit/munit.c
int main(int argc, char *argv[argc + 1]) {
    MunitTest tests[] = {
        { .name="/test_create_and_free", .test=test_create_and_free, },
        { .name="/test_put_and_get_int", .test=test_put_and_get_int,  },
        { .name="/test_put_and_get_string", .test=test_put_and_get_string,  },
        { .name="/test_update_value", .test=test_update_value,  },
        { .name="/test_delete_key", .test=test_delete_key,  },
        { .name="/test_put_and_get_str_key", .test=test_put_and_get_str_key,  },
        { .name="/test_put_and_get_bool_values", .test=test_put_and_get_bool_values,  },

        NULL_TEST,
    };

     MunitSuite suite = {
        "/hashmap", /* name */
        tests, /* tests */
        nullptr, /* suites */
        1, /* iterations */
        MUNIT_SUITE_OPTION_NONE /* options */
      };

    // test_put_and_get_bool_values(nullptr, nullptr);

    int result = {};
    result = munit_suite_main(&suite, nullptr, argc, argv);
    return result;

}
