// test_array_list.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/09 19:09:13 PDT

#include <locale.h>
#include <stdio.h>

#include "array_list.h"
#include "../testing_utils.h"

static MunitResult test_create_0(const MunitParameter params[], void* fixture) {
    // test all default arguments
    List *list;
    list = list_create(0, LIST_DEFAULT_VALUE_POLICY, MEM_DEFAULT_MALLOC_POLICY);

    // munit_assert_ptr_not_null(sct);
    // HashMap *map = sct->map;
    // munit_assert_ptr_not_null(map);
    // munit_assert_int(16, ==, map->num_buckets);
    // // expect SCT_DEFAULT_DATA_POLICIES == map->data_policies
    // munit_assert_true(equals_data_policies(SCT_DEFAULT_DATA_POLICIES, map->data_policies));
    // // expect == map->mem_policy
    // munit_assert_true(equals_mem_policy(MAP_DEFAULT_MALLOC_POLICY, map->mem_policy));
    list_destroy(list);

    return MUNIT_OK;
}


int main_test_array_list(int argc, char *argv[argc + 1]) {
    setlocale(LC_NUMERIC, "en_US.UTF-8");   // use user's system locale

    MunitTest tests[] = {
        munit_test(test_create_0),


        MUNIT_NULL_TEST,
    };

    // apply_fixture(tests, intstr_fixture, hashintstr_free);

    [[maybe_unused]]
         MunitSuite suite = {
        "/test_array_list", /* name */
        tests, /* tests */
        nullptr, /* suites */
        1, /* iterations */
        MUNIT_SUITE_OPTION_NONE /* options */
      };


    int result = 0;
    result = munit_suite_main(&suite, nullptr, argc, argv);


    return result;

}

#ifdef TEST_ARRAY_LIST
int main(int argc, char *argv[argc + 1]) {
    return main_test_array_list(argc, argv);

}
#endif
