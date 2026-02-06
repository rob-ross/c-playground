//
// Created by Rob Ross on 2/1/26.
//

#pragma once
#include <stdbool.h>
#include <stdio.h>

// expands to printf( fmt, ...)  but adds a newline to the end of fmt.
#define println(fmt, ...)               \
    do {                                \
        printf((fmt), ##__VA_ARGS__);   \
        putchar('\n');                  \
    } while (0)

typedef enum case_e { CASE_INSENSITIVE, CASE_SENSITIVE } Case;

#define STRING_BUFFER_CAPACITY 1024

struct string_buffer {
    size_t length;
    char str_buffer[STRING_BUFFER_CAPACITY];
};

static const size_t MAX_ARGS = 1024;  // max number of variadic arguments processed

/**
 * Concat the strings in the argument list and return a newly allocated string. The last argument must be NULL.
 *
 * @param str1 first char* to concat
 * @param const char * ... zero or more additional strings
 * @return a newly allocated string concatenation of the arugment list.
 */
char * sutil_concat_char(const char *str1, ...);

bool stuil_strings_equal(const char *str1, const char *str2);
bool sutil_strings_equal_case(const char *str1, const char *str2, const Case c);
bool sutil_strings_same(const char *s1, const char *s2);

char * sutil_zfill(const char* str, int width);

