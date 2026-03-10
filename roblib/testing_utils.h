//
// Created by Rob Ross on 2/26/26.
//

#pragma once

#include "./munit/munit.h"
#include "./munit/munit_overrides.h"

typedef struct UCharStr { unsigned char err; const char *expected;} UCharStr;

static const UCharStr NULL_ENUM_STR = {0, nullptr};  //sentinel null value object

static constexpr MunitTest MUNIT_NULL_TEST = { nullptr, nullptr, nullptr, nullptr,
    MUNIT_TEST_OPTION_NONE, nullptr };

#if !defined(print)
// expands to printf( fmt, ...)  but adds a newline to the end of fmt.
#define print(fmt, ...)                 \
    do {                                \
        printf((fmt), ##__VA_ARGS__);   \
        putchar('\n');                  \
    } while (0)
#endif

[[maybe_unused]]
static void apply_fixture(MunitTest tests[static 1], MunitTestSetup setup, MunitTestTearDown tear_down) {
    size_t test_index = 0;
    do {
        MunitTest *test = &tests[test_index++];
        // ReSharper disable once CppIncompatiblePointerConversion
        test->setup = setup;
        test->tear_down = tear_down;
    } while ( tests[test_index].name != nullptr );

}