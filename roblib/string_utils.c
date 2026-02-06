//
// Created by Rob Ross on 2/1/26.
//
// utility functions for ASCII strings.

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "string_utils.h"

#include <tgmath.h>


static inline size_t min_size(const size_t a, const size_t b) {
    return (a < b) ? a : b;
}




char * sutil_concat_char(const char *str1, ...){
    if ( !str1 ){
        return "";
    }

    const char * arg_list[SUTIL_MAX_ARGS];
    size_t arg_list_count = 0;
    arg_list[arg_list_count++] = str1; // add first non-variadic arg to array
    size_t total_length = strlen(str1);

    va_list args;
    va_start(args, str1);
    const char *next = NULL;

    // Loop until NULL sentinel is found or max arguments is reached
    while ( arg_list_count < SUTIL_MAX_ARGS && (next = va_arg(args, const char *)) != NULL) {
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

char * sutil_copy_char(const char *str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char *new_string = malloc(len + 1);
    if (!new_string) return NULL;
    strcpy(new_string, str);
    return new_string;
}


bool sutil_ends_with(const char *str, const char *suffix) {
    if (!str || !suffix) return false;
    const size_t str_len = strlen(str);
    const size_t suffix_len = strlen(suffix);
    if ( suffix_len > str_len) return false;
    for ( size_t index = 0; index < suffix_len; index++ ) {
        if (str[ str_len - index -1] != suffix[suffix_len - index - 1]) return false;
    }
    return true;
}


char * sutil_lower(const char *str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char *new_string = malloc(len + 1);
    if (!new_string) return NULL;
    for (int i = 0; i < len; ++i) {
        new_string[i] = (char)tolower((unsigned char)str[i]);
    }
    new_string[len] = '\0';
    return new_string;
}


char * sutil_pad_center(const char *str, const int width, const char fill_char) {
    if (!str) return NULL;
    const size_t string_length = strlen(str);
    if ( width <= 0 || width <= string_length) {
        return sutil_copy_char(str);
    }
    const size_t width_size = (size_t)width;
    char *new_string = malloc(width_size + 1);
    if (!new_string) return NULL;
    const size_t left_index = floor(width_size / 2.0 - string_length / 2.0);
    memset(new_string, fill_char, width_size);
    new_string[width_size] = '\0';
    memcpy(new_string + left_index, str, string_length );

    return new_string;
}


char * sutil_pad_left(const char *str, const int width, const char fill_char) {
    if (!str) return NULL;
    const size_t string_length = strlen(str);
    if ( width <= 0 || width <= string_length) {
        return sutil_copy_char(str);
    }
    const size_t width_size = (size_t)width;
    char *new_string = malloc(width_size + 1);
    if (!new_string) return NULL;
    const size_t left_index = width_size - string_length;
    memset(new_string, fill_char, left_index);
    strcpy(new_string + left_index, str);

    return new_string;
}

char * sutil_pad_right(const char *str, const int width, const char fill_char) {
    if (!str) return NULL;
    const size_t string_length = strlen(str);
    if ( width <= 0 || width <= string_length) {
        return sutil_copy_char(str);
    }
    const size_t width_size = (size_t)width;
    char *new_string = malloc(width_size + 1);
    if (!new_string) return NULL;

    strcpy(new_string, str);
    memset(new_string + string_length, fill_char, width_size - string_length);
    new_string[width_size] = '\0';

    return new_string;
}

bool sutil_starts_with(const char *str, const char *prefix) {
    if (!str || !prefix) return false;
    const size_t str_len = strlen(str);
    const size_t prefix_len = strlen(prefix);
    if ( prefix_len > str_len) return false;

    const char *next = prefix;
    size_t str_index = 0;
    while ( *next != '\0' && str[str_index] != '\0') {
        if (str[str_index++] != *next++ ) return false;
    }
    return true;
}

bool sutil_strings_equal(const char *str1, const char *str2) {
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
        return sutil_strings_equal(str1, str2);
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

char * sutil_upper(const char *str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char *new_string = malloc(len + 1);
    if (!new_string) return NULL;
    for (int i = 0; i < len; ++i) {
        new_string[i] = (char)toupper((unsigned char)str[i]);
    }
    new_string[len] = '\0';
    return new_string;
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
