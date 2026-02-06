//
// Created by Rob Ross on 2/1/26.
//

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "string_util.h"


static inline size_t min_size(const size_t a, const size_t b) {
    return (a < b) ? a : b;
}

/**
 * Concat the strings in the argument list and return a newly allocated string. The last argument must be NULL.
 *
 * @param str1 first char* to concat
 * @param const char * ... zero or more additional strings
 * @return a newly allocated string concatenation of the arugment list.
 */
char * sutil_concat_char(const char *str1, ...){
    if ( !str1 ){
        return "";
    }

    const char * arg_list[MAX_ARGS];
    size_t arg_list_count = 0;
    arg_list[arg_list_count++] = str1; // add first non-variadic arg to array
    size_t total_length = strlen(str1);

    va_list args;
    va_start(args, str1);
    const char *next = NULL;

    // Loop until NULL sentinel is found or max arguments is reached
    while ( arg_list_count < MAX_ARGS && (next = va_arg(args, const char *)) != NULL) {
        arg_list[arg_list_count++] = next;
        total_length += strlen(next);
    }
    va_end(args);

    char *new_str = malloc(sizeof(char) * total_length + 1);
    if (!new_str){
        return NULL;
    }

    new_str[0] = '\0'; // for the first strcat to work.
    for (int i = 0; i < arg_list_count; i++) {
        strcat(new_str, arg_list[i]);
    }
    return new_str;
}


bool stuil_strings_equal(const char *str1, const char *str2) {
    const size_t len1 = strlen(str1);
    const size_t len2 = strlen(str2);
    if (len1 != len2) {
        return false;
    }
    for (int i = 0; i < len1; ++i) {
        if (str1[i] != str2[i]) {
            return false;
        }
    }
    return true;
}

bool sutil_strings_equal_case(const char *str1, const char *str2, const Case c) {
    if (c == CASE_SENSITIVE) {
        return stuil_strings_equal(str1, str2);
    }
    const size_t len1 = strlen(str1);
    const size_t len2 = strlen(str2);
    if (len1 != len2) {
        return false;
    }
    for (int i = 0; i < len1; ++i) {
        const int c1 = toupper((unsigned char)str1[i]);
        const int c2 = toupper((unsigned char)str2[i]);
        if ( c1 != c2 ) {
            return false;
        }
    }
    return true;
}

bool sutil_strings_same(const char *s1, const char *s2) {
    return s1 == s2;
}

char * sutil_zfill(const char* str, int width){
    const size_t string_length = strlen(str);
    if (width <= string_length) {
        // Return a copy so the caller can safely free() the result
        char *copy = malloc(string_length + 1);
        if (copy) {
            strcpy(copy, str);
        }
        return copy;
    }

    char *new_str = malloc(width + 1);
    if (!new_str) return NULL;

    size_t padding = width - string_length;
    size_t offset = 0;

    // Handle sign
    if (str[0] == '-' || str[0] == '+') {
        new_str[0] = str[0];
        offset = 1;
    }

    memset(new_str + offset, '0', padding);
    strcpy(new_str + offset + padding, str + offset);

    return new_str;
}


#ifdef STRING_UTIL_TEST_MAIN
int main(void) {

    // Important: The list of arguments must be terminated with NULL
    printf("concatenated string: %s\n", sutil_concat_char("foo", "bar", "baz", NULL));

	return 0;
}
#endif
