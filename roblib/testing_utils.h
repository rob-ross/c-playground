//
// Created by Rob Ross on 2/26/26.
//

#pragma once

#include "./munit/munit.h"

typedef struct UCharStr { unsigned char err; const char *expected;} UCharStr;

static const UCharStr NULL_ENUM_STR = {0, nullptr};  //sentinel null value object

static const MunitTest NULL_TEST = { nullptr, nullptr, nullptr, nullptr,
    MUNIT_TEST_OPTION_NONE, nullptr };

#if !defined(print)
// expands to printf( fmt, ...)  but adds a newline to the end of fmt.
#define print(fmt, ...)                 \
    do {                                \
        printf((fmt), ##__VA_ARGS__);   \
        putchar('\n');                  \
    } while (0)
#endif