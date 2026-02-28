#include <locale.h>
#include <stdio.h>

#include "../testing_utils.h"
#include "hashmap.h"
#include <stdlib.h>
#include <string.h>

#include "../carray/carray_types.h"


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

void test_repr(void) {
    HashMap *map ;

    map = create_map(0, free);
    repr_HashMap(map, true); print("");
    free_map(map);

    map = create_map(10, free);
    repr_HashMap(map, true); print("");
    free_map(map);

    map = create_map(16, free);
    repr_HashMap(map, true); print("");
    free_map(map);

    map = create_map(17, free);

    free_map(map);
}

MunitResult test_10K_inserts(const MunitParameter params[], void* fixture) {
    HashMap *map ;
    const size_t N = 10'000;
    map = create_map(0, free);
    size_t buffer_size = 20;  // max 9 chars for value of i, plus 5 for 'hello', plus terminator
    for (int i = 0; i < N; ++i) {
        char search_string[buffer_size] = {};
        snprintf(search_string, buffer_size, "hello%d",i+1);
        map_put_klong_vstring(map, i, search_string );
    }
    for (int i=0; i< N; ++i) {
        char search_string[buffer_size] = {}; // max 4 chars for value of i, plus 5 for 'hello', plus terminator
        snprintf(search_string,buffer_size, "hello%d",i+1);

        const char* value = map_get_klong_vstring(map, i );
        // print("search string = %s, value = %s", search_string, value);

        munit_assert_string_equal( search_string, value);
    }

    print("");

    printf("finished adding %zu items to map.\n", N);
    repr_HashMap(map, false);

    free_map(map);

    return MUNIT_OK;
}


MunitResult test_klong_vstring(const MunitParameter params[], void* fixture) {
    HashMap *map ;

    map = create_map(0, free);
    for (int i = 0; i < 100; ++i) {
        char search_string[10]; // max 4 chars for value of i, plus 5 for 'hello', plus terminator
        snprintf(search_string, 10, "hello%d",i+1);
        map_put_klong_vstring(map, i, search_string );
    }
    for (int i=0; i< 100; ++i) {
        char search_string[10]; // max 4 chars for value of i, plus 5 for 'hello', plus terminator
        snprintf(search_string,10, "hello%d",i+1);

        const char* value = map_get_klong_vstring(map, i );
        // print("search string = %s, value = %s", search_string, value);

        munit_assert_string_equal( search_string, value);
    }

    print("");
    repr_HashMap(map, false);

    free_map(map);

    return MUNIT_OK;
}

MapValue value_for_long(const long v) {
    return (MapValue){.vlong = v, .value_type = MAP_TYPE_LONG};
}

MapValue value_for_double(const double v) {
    return (MapValue){.vdouble = v, .value_type = MAP_TYPE_DOUBLE};
}

MapValue value_for_string(char * v) {
    return (MapValue){.vstring = v, .value_type = MAP_TYPE_STRING};
}

MapValue value_for_void_ptr(void * v) {
    return (MapValue){.vvoid_ptr = v, .value_type = MAP_TYPE_VOID_PTR};
}

MapKey key_for_long(const long v) {
    return (MapKey){.klong = v, .key_type = MAP_TYPE_LONG};
}

MapKey key_for_double(const double v) {
    return (MapKey){.kdouble = v, .key_type = MAP_TYPE_DOUBLE};
}

MapKey key_for_string(char * v) {
    return (MapKey){.kstring = v, .key_type = MAP_TYPE_STRING};
}

MapKey key_for_void_ptr(void * v) {
    return (MapKey){.kvoid_ptr = v, .key_type = MAP_TYPE_VOID_PTR};
}

#define MAP_KEY(K) _Generic( (K),                                   \
    long: key_for_long(K), const long: key_for_long(K),             \
    double: key_for_double(K), const double: key_for_double(K),     \
    char *: key_for_string(K), const char *: key_for_string(K),     \
    void *: key_for_void_ptr(K), const void *: key_for_void_ptr(K)  \
    )

#define MAP_VALUE(V) _Generic( (V),                                       \
    long: value_for_long(V), const long: value_for_long(V),             \
    double: value_for_double(V), const double: value_for_double(V),     \
    char *: value_for_string(V), const char *: value_for_string(V),     \
    void *: value_for_void_ptr(V), const void *: value_for_void_ptr(V)  \
    )

void map_put(HashMap *map, MapKey k, MapValue v);

#define map_put_( M, K, V ) map_put( (M), (K), (V) )
#define map_put(M, K, V) map_put_( (M), MAP_KEY(K), MAP_VALUE(V) )


void test_arg_macros(void){
    HashMap *map = create_map(0, free);

    map_put(map, 42, "foo");
        
}


// make
// clang -std=c23 -o ./out/test_hashmap.out test_hashmap.c hashmap.c ../munit/munit.c
int main(int argc, char *argv[argc + 1]) {
    setlocale(LC_NUMERIC, "");   // use user's system locale
    printf("%'d\n", 1'000'000);

    MunitTest tests[] = {
        { .name="/test_create_and_free", .test=test_create_and_free, },
        { .name="/test_put_and_get_int", .test=test_put_and_get_int,  },
        { .name="/test_put_and_get_string", .test=test_put_and_get_string,  },
        { .name="/test_update_value", .test=test_update_value,  },
        { .name="/test_delete_key", .test=test_delete_key,  },
        { .name="/test_put_and_get_str_key", .test=test_put_and_get_str_key,  },
        { .name="/test_put_and_get_bool_values", .test=test_put_and_get_bool_values,  },
        {.name="/test_klong_vstring", .test=test_klong_vstring },
        NULL_TEST,
    };


     MunitSuite suite = {
        "/hashmap", /* name */
        tests, /* tests */
        nullptr, /* suites */
        1, /* iterations */
        MUNIT_SUITE_OPTION_NONE /* options */
      };

    test_10K_inserts(nullptr, nullptr);

    int result = {};
    // result = munit_suite_main(&suite, nullptr, argc, argv);
    return result;

}
