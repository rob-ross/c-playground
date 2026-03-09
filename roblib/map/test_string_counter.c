// test_string_counter.c
//
// Copyright (c) Rob Ross 2026. 
//
//
// Created 2026/03/03 22:32:05 PST

#include <locale.h>
#include <stdio.h>

#include "string_counter.h"
#include "../testing_utils.h"
#include "hashmap.h"

struct StringCounter {
    HashMap *map;
};

// -----------------------------------------------
// setup and teardown fixtures
// ----------------------------------------------
// create a HashMap for use in test cases
static void *  intstr_fixture(const MunitParameter params[], void* user_data) {
    StringCounter *sct = sct_create(16);
    munit_assert_not_null(sct);
    HashMap *map = sct->map;
    munit_assert_not_null(map);
    return sct;
}

// to free the hashmap created by the intstr_fixture after a test
static void hashintstr_free(void * fixture) {
    sct_destroy((StringCounter*)fixture);
}

static bool equals_data_policies(const MapDataPolicies o1, const MapDataPolicies o2) {
    const MapKeyPolicy kp1 = o1.key_policy;
    const MapKeyPolicy kp2 = o2.key_policy;
    if (kp1.context != kp2.context ||
        kp1.on_add_key != kp2.on_add_key ||
        kp1.on_free_key != kp2.on_free_key ||
        kp1.on_free_context != kp2.on_free_context) {
        return false;
    }
    const MapValuePolicy vp1 = o1.value_policy;
    const MapValuePolicy vp2 = o2.value_policy;
    if (vp1.context != vp2.context ||
        vp1.on_set_value != vp2.on_set_value ||
        vp1.on_free_value != vp2.on_free_value ||
        vp1.on_free_context != vp2.on_free_context) {
        return false;
        }
    return true;
}

static bool equals_mem_policy(const MemPolicy o1, const MemPolicy o2) {
    if (o1.context != o2.context ||
        o1.alloc != o2.alloc ||
        o1.free != o2.free ||
        o1.free_context != o2.free_context ||
        o1.policy_type != o2.policy_type ) {
        return false;
    }
    return true;
}

// -------------------------------------------------
// test cases
// -------------------------------------------------

static MunitResult test_create_0(const MunitParameter params[], void* fixture) {
    // test all default arguments
    StringCounter *sct;
    sct = sct_create();
    munit_assert_ptr_not_null(sct);
    HashMap *map = sct->map;
    munit_assert_ptr_not_null(map);
    munit_assert_int(16, ==, map->num_buckets);
    // expect SCT_DEFAULT_DATA_POLICIES == map->data_policies
    munit_assert_true(equals_data_policies(SCT_DEFAULT_DATA_POLICIES, map->data_policies));
    // expect == map->mem_policy
    munit_assert_true(equals_mem_policy(MAP_DEFAULT_MALLOC_POLICY, map->mem_policy));
    sct_destroy(sct);
    return MUNIT_OK;
}

static MunitResult test_create_1(const MunitParameter params[], void* fixture) {
    // test passing num_buckets argument
    StringCounter *sct;
    sct = sct_create(0);
    munit_assert_int(16, ==, sct->map->num_buckets);
    munit_assert_true(equals_data_policies(SCT_DEFAULT_DATA_POLICIES, sct->map->data_policies));
    munit_assert_true(equals_mem_policy(MAP_DEFAULT_MALLOC_POLICY, sct->map->mem_policy));

    sct_destroy(sct);

    sct = sct_create(10);  //closest power of 2 is 16
    munit_assert_int(16, ==, sct->map->num_buckets);
    munit_assert_true(equals_data_policies(SCT_DEFAULT_DATA_POLICIES, sct->map->data_policies));
    munit_assert_true(equals_mem_policy(MAP_DEFAULT_MALLOC_POLICY, sct->map->mem_policy));
    sct_destroy(sct);

    sct = sct_create(20);  //closest power of 2 is 32
    munit_assert_int(32, ==, sct->map->num_buckets);
    munit_assert_true(equals_data_policies(SCT_DEFAULT_DATA_POLICIES, sct->map->data_policies));
    munit_assert_true(equals_mem_policy(MAP_DEFAULT_MALLOC_POLICY, sct->map->mem_policy));
    sct_destroy(sct);

    return MUNIT_OK;
}

static MapKey dummy_on_add_key(HashMap *map, MapKey key) {
    return (MapKey){};
}

static void dummy_on_free_key(HashMap *map, MapKey key) {}

static void dummy_on_free_context(void* context) {}


static MapValue dummy_on_set_value(HashMap *map, MapValue value) {
    return (MapValue){};
}

static void dummy_on_free_value(HashMap *map, MapValue value){}

int dummy_int = 42;
void * dummy_context = &dummy_int;

static MapDataPolicies data_policy_fixture() {
    return (MapDataPolicies){
        .key_policy={
            .context = dummy_context, .on_add_key   = dummy_on_add_key, .on_free_key = dummy_on_free_key,
                .on_free_context = dummy_on_free_context, .policy_type = MAP_POLICY_SHARED},
        .value_policy ={
            .context = dummy_context, .on_set_value = dummy_on_set_value,  .on_free_value = dummy_on_free_value,
            .on_free_context = dummy_on_free_context, .policy_type = MAP_POLICY_SHARED}
    };
}

void * dummy_alloc( void * context, size_t num_bytes ) { return calloc(1, num_bytes); }
void dummy_free(  void * context, void * bytes){ free(bytes); }
void dummy_free_context(void * context ) {}


static MemPolicy mem_policy_fixture() {
    return (MemPolicy){
        .context = dummy_context, .alloc = dummy_alloc, .free = dummy_free, .free_context = dummy_free_context,
        .policy_type = MEM_POLICY_MALLOC_SHARED
    };
}

static MunitResult test_create_2(const MunitParameter params[], void* fixture) {
    // test passing num_buckets,  MapDataPolicies argument, default MemPolicy
    const MapDataPolicies data_policy = data_policy_fixture();
    StringCounter *sct;
    sct = sct_create(0, data_policy);
    munit_assert_int(16, ==, sct->map->num_buckets);
    munit_assert_true(equals_data_policies(data_policy, sct->map->data_policies));
    munit_assert_true(equals_mem_policy(MAP_DEFAULT_MALLOC_POLICY, sct->map->mem_policy));
    sct_destroy(sct);

    return MUNIT_OK;
}

static MunitResult test_create_3(const MunitParameter params[], void* fixture) {
    // test passing num_buckets, MapDataPolicies, and MemPolicy arguments
    const MapDataPolicies data_policy = data_policy_fixture();
    const MemPolicy       mem_policy = mem_policy_fixture();

    StringCounter *sct;
    sct = sct_create(0, data_policy, mem_policy);
    munit_assert_int(16, ==, sct->map->num_buckets);
    munit_assert_true(equals_data_policies(data_policy, sct->map->data_policies));
    munit_assert_true(equals_mem_policy(mem_policy, sct->map->mem_policy));
    sct_destroy(sct);

    return MUNIT_OK;
}

static MunitResult test_clear(const MunitParameter params[], void* fixture) {
    StringCounter *map = fixture;

    sct_put(map,"dog", 1);
    sct_put(map, "cat", 1);
    munit_assert_int(2, ==,  map->map->size);
    sct_clear( map );
    munit_assert_int(0, ==,  map->map->size);
    munit_assert_true(sct_is_empty(map));
    return MUNIT_OK;
}

static MunitResult test_contains_key(const MunitParameter params[], void* fixture) {
    StringCounter *map = fixture;

    sct_put(map,"dog",1);
    sct_put(map, "cat", 1);
    sct_put(map, "wolf",1 );

    munit_assert_true(sct_contains_key(map, "dog"));
    munit_assert_true(sct_contains_key(map, "cat"));
    munit_assert_true(sct_contains_key(map, "wolf"));
    munit_assert_false(sct_contains_key(map, "no such key"));

    return MUNIT_OK;
}

static MunitResult test_get(const MunitParameter params[], void* fixture) {
    StringCounter *map = fixture;
    MapValue value;

    sct_put(map,"dog",1);
    value = sct_get(map, key_for_string("dog"));
    munit_assert_int(1, ==, value.vlong);

    sct_put(map,"dog",11);
    value = sct_get(map, key_for_string("dog"));
    munit_assert_int(11, ==, value.vlong);

    return MUNIT_OK;
}

static MunitResult test_get_count(const MunitParameter params[], void* fixture) {
    StringCounter *map = fixture;
    long count;

    sct_put(map,"dog",1);
    count = sct_get_count(map, "dog");
    munit_assert_int(1, ==, count);

    sct_put(map,"dog",11);
    count = sct_get_count(map, "dog");
    munit_assert_int(11, ==, count);

    return MUNIT_OK;
}

static MunitResult test_is_empty(const MunitParameter params[], void* fixture) {
    StringCounter *map = fixture;
    sct_put(map,"dog", 1);
    sct_put(map, "cat", 1);
    munit_assert_false(sct_is_empty(map));

    sct_clear(map);
    munit_assert_true(sct_is_empty(map));

    return MUNIT_OK;
}

static MunitResult test_put(const MunitParameter params[], void* fixture) {
    StringCounter *map = fixture;
    sct_put(map,"dog", 1);
    sct_put(map, "cat", 22);

    munit_assert_int(1, ==, sct_get_count(map, "dog"));
    munit_assert_int(22, ==, sct_get_count(map, "cat"));
    munit_assert_int(2, ==, sct_size(map));

    return MUNIT_OK;

}

static MunitResult test_ref(const MunitParameter params[], void* fixture) {
    StringCounter *map = fixture;

    munit_assert_int(0, ==, sct_get_count(map, "dog"));

    char const *dog_literal = "dog";
    char const *dog_ref = sct_ref(map, "dog");

    munit_assert_not_null(dog_ref);
    munit_assert_ptr_not_equal(dog_literal, dog_ref);
    munit_assert_string_equal(dog_literal, dog_ref);
    munit_assert_int(1, ==, sct_get_count(map, "dog"));

    sct_ref(map, "dog");
    sct_ref(map, "dog");
    munit_assert_int(3, ==, sct_get_count(map, "dog"));

    return MUNIT_OK;

}

static MunitResult test_unref(const MunitParameter params[], void* fixture) {
    StringCounter *map = fixture;

    sct_put(map, "dog", 3);
    munit_assert_int(3, ==, sct_get_count(map, "dog"));
    sct_unref(map, "dog");
    sct_unref(map, "dog");

    munit_assert_int(1, ==, sct_get_count(map, "dog"));
    sct_unref(map, "dog");
    munit_assert_int(0, ==, sct_get_count(map, "dog"));
    munit_assert_false(sct_contains_key(map, "dog"));

    return MUNIT_OK;
}

static MunitResult test_remove(const MunitParameter params[], void* fixture) {
    StringCounter *map = fixture;
    sct_put(map,"dog", 1);
    sct_put(map, "cat", 22);

    munit_assert_int(1, ==, sct_get_count(map, "dog"));
    munit_assert_int(22, ==, sct_get_count(map, "cat"));
    munit_assert_int(2, ==, sct_size(map));

    sct_remove(map, "dog");
    munit_assert_int(0, ==, sct_get_count(map, "dog"));
    munit_assert_false(sct_contains_key(map, "dog"));

    munit_assert_int(22, ==, sct_get_count(map, "cat"));
    munit_assert_int(1, ==, sct_size(map));

    return MUNIT_OK;
}

static MunitResult test_size(const MunitParameter params[], void* fixture) {
    StringCounter *map = fixture;
    sct_put(map,"dog", 1);
    sct_put(map, "cat", 22);

    munit_assert_int(1, ==, sct_get_count(map, "dog"));
    munit_assert_int(22, ==, sct_get_count(map, "cat"));
    munit_assert_int(2, ==, sct_size(map));

    sct_remove(map, "dog");
    munit_assert_int(1, ==, sct_size(map));

    sct_remove(map, "cat");
    munit_assert_int(0, ==, sct_size(map));

    return MUNIT_OK;
}


// this should be part of a performance test, i.e., complete method in under X time.
[[maybe_unused]]
static MunitResult test_100_string_different(const MunitParameter params[], void* fixture) {
    StringCounter *map = fixture;
    for (int i = 0; i < 100; ++i) {
        char strkey[10]; // max 4 chars for value of i, plus 5 for 'hello', plus terminator
        snprintf(strkey, 10, "hello%d",i+1);
        sct_put(map, strkey, i );
    }
    munit_assert_int(100, ==, sct_size(map));

    for (int i=0; i< 100; ++i) {
        char search_string[10]; // max 4 chars for value of i, plus 5 for 'hello', plus terminator
        snprintf(search_string,10, "hello%d",i+1);

        long vlong = sct_get_count(map, search_string );

        munit_assert_int( 1, ==, vlong);
    }

    return MUNIT_OK;
}

// ------------------------------------
// end test cases
// ------------------------------------


static void apply_fixture(MunitTest tests[static 1], MunitTestSetup setup, MunitTestTearDown tear_down) {
    size_t test_index = 0;
    do {
        MunitTest *test = &tests[test_index++];
        // ReSharper disable once CppIncompatiblePointerConversion
        test->setup = setup;
        test->tear_down = tear_down;
    } while ( tests[test_index].name != nullptr );


}
// make
// clang -std=c23 -fsanitize=address -fsanitize=leak -Wall -Werror -o ./out/test_string_counter.out test_string_counter.c string_counter.c hashmap.c ../memory/memory_pool.c ../munit/munit.c
//
//  clang -std=c23 -Wall -Werror -o ./out/test_string_counter.out test_string_counter.c string_counter.c hashmap.c ../memory/memory_pool.c ../munit/munit.c
//
int main_test_string_counter(int argc, char *argv[argc + 1]) {
    setlocale(LC_NUMERIC, "en_US.UTF-8");   // use user's system locale

    MunitTest tests[] = {
        munit_test(test_create_0),
        munit_test(test_create_1),
        munit_test(test_create_2),
        munit_test(test_create_3),
        munit_test(test_clear),
        munit_test(test_contains_key),
        munit_test(test_get),
        munit_test(test_get_count),
        munit_test(test_is_empty),
        munit_test(test_put),
        munit_test(test_ref),
        munit_test(test_unref),
        munit_test(test_remove),
        munit_test(test_size),

        MUNIT_NULL_TEST,
    };

    apply_fixture(tests, intstr_fixture, hashintstr_free);

[[maybe_unused]]
     MunitSuite suite = {
        "/test_string_counter", /* name */
        tests, /* tests */
        nullptr, /* suites */
        1, /* iterations */
        MUNIT_SUITE_OPTION_NONE /* options */
      };


    int result = 0;
    result = munit_suite_main(&suite, nullptr, argc, argv);


    // test_create_3(nullptr, nullptr);


    return result;

}

#ifdef TEST_STRING_COUNTER
int main(int argc, char *argv[argc + 1]) {
    return main_test_string_counter(argc, argv);

}
#endif
