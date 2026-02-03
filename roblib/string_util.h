//
// Created by Rob Ross on 2/1/26.
//

#pragma once
#include <stdbool.h>

// expands to printf( fmt, ...)  but adds a newline to the end of fmt.
#define println(fmt, ...)               \
    do {                                \
        printf((fmt), ##__VA_ARGS__);   \
        putchar('\n');                  \
    } while (0)

typedef enum case_e { CASE_INSENSITIVE, CASE_SENSITIVE } Case;

bool strings_equal(const char *str1, const char *str2);
bool strings_equal_case(const char *str1, const char *str2, Case c);
bool strings_same(const char *s1, const char *s2);