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

static bool equals_ListValuePolicy(ListValuePolicy o1, ListValuePolicy o2) {
    return memcmp(&o1, &o2, sizeof(ListValuePolicy)) == 0;
}

bool equals_MemPolicy(MemPolicy o1, MemPolicy o2) {
    return memcmp(&o1, &o2, sizeof(MemPolicy)) == 0;
}

// -----------------------------------------------
// setup and teardown fixtures
// ----------------------------------------------
// create a HashMap for use in test cases
static void *  list_fixture(const MunitParameter params[], void* user_data) {
    List *list = list_create(0, LIST_DEFAULT_VALUE_POLICY, MEM_DEFAULT_MALLOC_POLICY);
    munit_assert_not_null(list);
    return list;
}

// to free the hashmap created by the hashmap_fixture after a test
static void list_free(void * fixture) {
    list_destroy(fixture);
}

// -------------------------------------------------
// test cases
// -------------------------------------------------
static MunitResult test_create_3(const MunitParameter params[], void* fixture) {
    // test all default arguments
    List *list;
    list = list_create(0, LIST_DEFAULT_VALUE_POLICY, MEM_DEFAULT_MALLOC_POLICY);

    munit_assert_ptr_not_null(list);
    munit_assert_int(0, ==, list->size);
    munit_assert_int(LIST_MIN_CAPACITY, ==, list->capacity);
    munit_assert_true(list_is_empty(list));

    // expect LIST_DEFAULT_VALUE_POLICY == list->value_policy
    munit_assert_true(equals_ListValuePolicy(LIST_DEFAULT_VALUE_POLICY, list->value_policy));
    // expect == list->mem_policy
    munit_assert_true(equals_MemPolicy(MEM_DEFAULT_MALLOC_POLICY, list->mem_policy));
    list_destroy(list);

    return MUNIT_OK;
}

static MunitResult test_append(const MunitParameter params[], void* fixture) {
    List *list = fixture;

    CollectionsError result = list_append(list, (ListValue){.vlong = 1, .value_type = LIST_TYPE_LONG});
    munit_assert_int(COL_OK, ==, result);

    munit_assert_int(1, ==, list_size(list));

    ListValue v = list_get(list, 0);
    munit_assert_int(1, ==, v.vlong);

    return MUNIT_OK;
}

int main_test_array_list(int argc, char *argv[argc + 1]) {
    setlocale(LC_NUMERIC, "en_US.UTF-8");   // use user's system locale

    MunitTest tests[] = {
        munit_test(test_create_3),
        munit_test(test_append),


        MUNIT_NULL_TEST,
    };

    apply_fixture(tests, list_fixture, list_free);

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

    // test_append(nullptr, list_fixture(nullptr, nullptr));

    return result;

}

#ifdef TEST_ARRAY_LIST
int main(int argc, char *argv[argc + 1]) {
    return main_test_array_list(argc, argv);

}
#endif
