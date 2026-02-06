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

static const size_t SUTIL_MAX_ARGS = 1024;  // max number of variadic arguments processed





/**
 * Concatenates the strings in the argument list and returns a newly allocated string. The last argument must be NULL.
 *
 * @param str1 first const char* to concat
 * @param ... const char* zero or more additional strings
 * @return a newly allocated string concatenation of the arugment list.
 */
char * sutil_concat_char(const char *str1, ...);
/**
 * Returns a newly allocated string that is a copy of the input string.
 * @param str input string
 * @return a newly allocated string copy of the original string
 */
char * sutil_copy_char(const char *str);


/**
 * Returns true if the input string ends with the specified suffix, otherwise return false.
 * @param str the input string
 * @param suffix the substring to match at the end of the input string
 * @return true if `str` ends with `suffix`, else false.
 */
bool sutil_ends_with(const char *str, const char *suffix);

/**
 * Returns a new copy of the string with all the cased characters converted to lowercase.
 * Operates by calling tolower() on each character of the input string.
 * @param str ASCII string
 * @return a new copy of the string with all the cased characters converted to lowercase.
 */
char * sutil_lower(const char *str) ;



/**
 * Returns a new string with argument `str` centered in a string of length width.
 * Padding is done using the specified fill_char.
 * A copy of the original string is returned if width is less than or equal to len(s).
 * @param str source string to center fill
 * @param width length of new padded string
 * @param fill_char the character to use for padding
 * @return newly allocated string.
 */
char * sutil_pad_center(const char* str, int width, char fill_char);

/**
 * Returns a newly allocated string that is size `width`, left-padded with the fill character.
 * If `width` is <= strlen(str), returns a new copy of the argument string (unchanged.).
 * @param str source string to left fill
 * @param width length of new padded string
 * @param fill_char the character to use for padding
 * @return newly allocated string.
 */
char * sutil_pad_left(const char *str, int width, char fill_char);

/**
 * Returns a newly allocated string that is size `width`, right-padded with the fill character.
 * If `width` is <= strlen(str), returns a new copy of the argument string (unchanged.).
 * @param str source string to right fill
 * @param width length of new padded string
 * @param fill_char the character to use for padding
 * @return newly allocated string.
 */
char * sutil_pad_right(const char *str,int width, char fill_char);


/**
 * Returns true if the input string starts with the specified prefix, otherwise return false.
 * @param str the input string
 * @param prefix the substring to match at the start of the input string
 * @return true if `str` starts with `prefix`, else false.
 */
bool sutil_starts_with(const char *str, const char *prefix);

bool sutil_strings_equal(const char *str1, const char *str2);
bool sutil_strings_equal_case(const char *str1, const char *str2, Case c);
bool sutil_strings_same(const char *s1, const char *s2);


/**
 * Returns a new copy of the string with all the cased characters converted to uppercase.
 * Operates by calling toupper() on each character of the input string.
 * @param str ASCII string
 * @return a new copy of the string with all the cased characters converted to uppercase.
 */
char * sutil_upper(const char *str);

char * sutil_zfill(const char* str, int width);
