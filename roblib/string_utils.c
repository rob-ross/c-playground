//
// Created by Rob Ross on 2/1/26.
//
// utility functions for ASCII strings.

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "string_utils.h"


static inline size_t min_size(const size_t a, const size_t b) {
    return (a < b) ? a : b;
}

/**
 * Returns true if the char `c` is found in the `chars` array.
 * @param c the character to check
 * @param chars the characters to check against
 * @return true if char `c` is found in `chars`, otherwise return false.
 */
bool sutil_char_in(const char c, const char *chars) {
    if (!chars) return false;
    const size_t chars_len = strlen(chars);
    for (size_t i = 0; i < chars_len; ++i) {
        if (c == chars[i]) return true;
    }
    return false;
}

char * sutil_concat_char(const char *str1, ...){
    if ( !str1 ) return sutil_copy_char("");

    const char * arg_list[SUTIL_MAX_ARGS];
    size_t arg_lengths[SUTIL_MAX_ARGS];
    size_t arg_list_count = 0;
    arg_lengths[arg_list_count] = strlen(str1);
    arg_list[arg_list_count++] = str1; // add first non-variadic arg to array
    size_t total_length = arg_lengths[0];

    va_list args;
    va_start(args, str1);
    const char *next = NULL;

    // Loop until NULL sentinel is found or max arguments is reached
    while ( arg_list_count < SUTIL_MAX_ARGS && (next = va_arg(args, const char *)) != NULL) {
        size_t len = strlen(next);
        arg_lengths[arg_list_count] = len;
        arg_list[arg_list_count++] = next;
        total_length += len;
    }
    va_end(args);

    char *new_str = malloc(sizeof(char) * total_length + 1);
    if (!new_str){
        return NULL;
    }

    char *dest = new_str;
    for (int i = 0; i < arg_list_count; i++) {
        memcpy(dest, arg_list[i], arg_lengths[i]);
        dest += arg_lengths[i];
    }
    *dest = '\0';
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

int sutil_index_start_end_impl_(
    const char *str,
    const char *substring,
    const size_t start,
    const size_t end,
    const size_t str_len,
    const size_t substring_len) {
    // printf("sutil_index_start_end_(%s, %s, %zu, %zu) \n", str, substring, start, end);

    if (! str || !substring ) {
        return -1;
    }

    const size_t max_index = str_len - substring_len; // result index can't be greater than this

    // printf("str_len: %zu, substring_len: %zu, max_index: %zu\n", str_len, substring_len, max_index);

    if (str_len < substring_len || str_len == 0 || str_len > INT_MAX ||
             substring_len == 0 || start > max_index || end >  str_len ) {
        return -1;  // we don't count empty substring as a substring of any string.
             }

    // printf("Got past all if checks.\n");

    // find start of possible match
    for (size_t i = start; i <= end; i++) {
        if ( strncmp(&str[i], substring, substring_len) == 0 ) {
            if (i <= INT_MAX) {
                return (int)i;
            }
            return -1;  // index too big to fit in signed int.
        }
    }

    return -1;
}

int sutil_index_start_end_(const char *str, const char *substring, const size_t start, const size_t end) {
    return sutil_index_start_end_impl_(str, substring, start, end, strlen(str), strlen(substring));
}

// make:   clang -DSUTIL_TEST_MAIN -o string_utils.out string_utils.c
#ifdef SUTIL_TEST_MAIN
int main(void) {
    char *str = "this is my test string";
    char *subs = "test";
    int actual = sutil_index_(str, subs);
    printf("called sutil_index_(%s, %s), returns: %d\n", str, subs,actual);



    return 0;
}
#endif


int sutil_index_start_(const char *str, const char *substring, const size_t start) {
    const size_t str_len = strlen(str);
    return sutil_index_start_end_impl_(str, substring, start, str_len, str_len, strlen(substring));
}

// return the index of the first occurrence of `sub_string` in `str`, or -1 if `substring not found
// only works on strings ~2GB max.
// todo naming? index_of? find?
int sutil_index_(const char *str, const char *substring) {
    const size_t str_len = strlen(str);
    return sutil_index_start_end_impl_(str, substring, 0, str_len, str_len, strlen(substring));
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
    const size_t left_index = (width_size - string_length) / 2;
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


char * sutil_strip(const char *str, const char *chars) {
    if (!str) return NULL;

    const size_t str_len = strlen(str);
    const char *char_set;
    if (!chars || chars[0] == '\0' ) char_set = SUTIL_WHITESPACE;
    else char_set = chars;
    //find left index of first non-stripped character
    size_t left_index = 0;
    for ( left_index = 0; left_index < str_len; ++left_index) {
        if (!sutil_char_in(str[left_index], char_set)) {
            break;
        }
    }

    if (left_index == str_len) {
        return sutil_copy_char("");
    }

    //find position of first non-stripped character. This is relative to the end of the list counting backwards
    size_t end = 0;
    for ( end = 0; end < str_len; ++end) {
        if (!sutil_char_in(str[str_len - end - 1], char_set)) {
            break;
        }
    }
    
    size_t new_len = str_len - left_index - end;
    char *new_str = malloc( new_len + 1 );
    if (!new_str) return NULL;
    memcpy(new_str, str + left_index, new_len);
    new_str[new_len] = '\0';
    return new_str;
}


char * sutil_strip_left(const char *str, const char *chars) {
    if (!str)  return NULL;
    const size_t str_len = strlen(str);
    const char *char_set;
    if (!chars || chars[0] == '\0' ) char_set = SUTIL_WHITESPACE;
    else char_set = chars;

    //find position of first non-stripped character
    size_t index = 0;
    for ( index = 0; index < str_len; ++index) {
        if (!sutil_char_in(str[index], char_set)) {
            break;
        }
    }
    char *new_str = malloc(str_len - index + 1);
    if (!new_str) return NULL;
    strcpy(new_str, str + index);

    return new_str;
}

char * sutil_strip_right(const char *str, const char *chars) {
    if (!str) return NULL;
    const size_t str_len = strlen(str);
    const char *char_set;
    if (!chars || chars[0] == '\0' ) char_set = SUTIL_WHITESPACE;
    else char_set = chars;

    //find last position of first non-stripped character
    size_t index = 0;
    for ( index = 0; index < str_len; ++index) {
        if (!sutil_char_in(str[str_len - index - 1], char_set)) {
            break;
        }
    }
    size_t new_len = str_len - index;
    char *new_str = malloc(new_len + 1);
    if (!new_str) return NULL;
    memcpy(new_str, str, new_len);
    new_str[new_len] = '\0';
    return new_str;
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
    if (!str) return NULL;
    const size_t string_length = strlen(str);
    if (width < 0 || (size_t)width <= string_length) {
        // Return a copy so the caller can safely free() the result
        char *copy = malloc(string_length + 1);
        if (copy) {
            strcpy(copy, str);
        }
        return copy;
    }

    char *new_str = malloc((size_t)width + 1);
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
