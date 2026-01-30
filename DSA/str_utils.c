//
// Created by Rob Ross on 1/27/26.
//
#include "str_utils.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <tgmath.h>

#include "range.h"

// make: clang -DSTR_UTILS_TEST_MAIN str_utils.c range.c -o str_util

/**
 * Concat the two strings and return in new char * buffer. Caller must free the memory.
 * @param str1 first string
 * @param str2 second string
 * @return a newly created buffer with the contents of both strings concatenated and terminated with \0
 */
char *concat(const char *str1, const char *str2) {

    const size_t str1_len = strlen(str1);
    const size_t str2_len = strlen(str2);

    const size_t buffer_size = str1_len + str2_len + 1;

    char *buffer = malloc( sizeof(char) * buffer_size);
    if (buffer == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    strcpy( buffer, str1);
    strcpy( buffer + str1_len, str2);

    // can I return a stack object? no, not dynamically.


    return buffer;

}

/**
 * Catenate str2 to the end of str1
 * @param str1
 * @param str2
 */
void concat_strings(char *str1, const char *str2) {
    //find null terminator of str1
    int str1_len = strlen(str1);
    printf("\nstr1='%s', len=%d, ", str1, str1_len);
    char *start_ptr = str1 + str1_len;
    int index = 0;
    printf("*(start_ptr - 1 )= '%c', *start-ptr='%c'\n", *(start_ptr - 1 ), *start_ptr );
    while ( str2[index] != '\0' ){
        start_ptr[index] = str2[index];
        index++;
    }
    start_ptr[index] = '\0';
}

/*
 * Return a string representing the boolean, 'false` for 0 and `true` for 1
 */
const char *bstr(const bool b) {
    return b == 0 ? "false" : "true";
}

/**
 * Converts an int to a newly allocated NULL-terminated string.
 * The returned pointer must be freed by the caller.
 * Returns: Pointer to heap-allocated string on success, or NULL on failure.
 * Ownership: Caller owns the returned string and must free() it.
 */
char *itostr(const int i) {

    // min int with separators requires 14 characters +\0: "-2,147,483,647"

    const char *format_string = "%i";

    // how many bytes will we need for our buffer?
    // snprintf returns int; negative means error
    const int needed = snprintf(NULL, 0, format_string, i);
    if (needed < 0) {
        return NULL;
    }

    char *buffer = malloc((size_t) needed + 1);  // +1 for string null terminator char
    if (buffer == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    const int bytes_written = snprintf(buffer, (size_t) needed + 1, format_string, i);
    if (bytes_written < 0 ) {
        //  formatting failed
        free(buffer);
        return NULL;
    }
    // If needed came from snprintf(NULL,0,...), this should be true on success
    if (bytes_written != needed) {
        free(buffer);
        return NULL;
    }

    return buffer;
}

char *empty_array_str(void) {
    const size_t buffer_size = (size_t)2 + 1;
    char *buffer = malloc(buffer_size);  // todo free me
    if (buffer == NULL) {
        printf("WARNING: malloc returned NULL in empty_array_str()\n");
        errno = ENOMEM;
        return NULL;
    }
    int r = snprintf(buffer, buffer_size, "[]");
    if (r < 0) {
        printf("Error in empty_array_str(). snprintf returned %i: r = snprintf(buffer, buffer_size, \"[]\");\n", r);
        free(buffer);
        return NULL;
    }
    return buffer;
}

/**
 * Converts an int[] to a newly allocated NULL-terminated string representing the contents of the array.
 * The returned pointer must be freed by the caller.
 * Returns: Pointer to heap-allocated string on success, or NULL on failure.
 * Ownership: Caller owns the returned string and must free() it.
 */
char *astr(const int nums[], const size_t nums_size) {
    if ( nums == NULL || nums_size == 0 ) {
        return empty_array_str();  //empty list
    }

    const char *format_string = "%i%s";
    const char *join_string = ", ";

    // 1. determine size required for concatenated string
    size_t total_bytes_needed = 0;

    const char *join = join_string;

    for (size_t i = 0; i < nums_size; i++ ) {
        if ( i == nums_size -1 ) join = "";
        int needed = snprintf(NULL, 0, format_string, nums[i], join);
        if (needed < 0 ) {
            printf("WARNING: snprintf returned %i in astr()\n", needed);
            return NULL;
        }
        total_bytes_needed += (size_t)needed;
    }

    total_bytes_needed += 4;  // for '[ ' ' ]' strings
    const size_t buffer_size = total_bytes_needed + 1;
    char *buffer = malloc(buffer_size);  // todo free me
    if (buffer == NULL) {
        printf("WARNING: malloc returned NULL in astr()\n");
        errno = ENOMEM;
        return NULL;
    }

    size_t used = 0;
    int r = 0;
    join = join_string;

    r = snprintf(buffer + used, buffer_size - used, "[ ");  // write initial '[ '
    if (r < 0) {
        printf("Error in astr(). snprintf returned %i: snprintf(buffer + used, buffer_size - used, \"[ \");\n", r);
        free(buffer);
        return NULL;
    }

    used += (size_t)r;

    for (size_t i = 0; i < nums_size; i++ ) {
        if ( i == nums_size -1 ) join = "";
        r = snprintf(buffer + used, buffer_size - used, format_string, nums[i], join);
        if (r < 0) {
            printf("Error in astr(). snprintf returned %i: r = snprintf(buffer + used, buffer_size - used, format_string, nums[i], join);\n", r);
            free(buffer);
            return NULL;
        }
        size_t ur = (size_t)r;
        if ( ur >= buffer_size -used ) {
            printf("Truncation Error in astr(). snprintf returned %i: r = snprintf(buffer + used, buffer_size - used, format_string, nums[i], join);\n", r);
            free(buffer);
            return NULL;
        }
        used += ur;
    }

    r =  snprintf(buffer + used, buffer_size - used, " ]");  // write closing ' ]'
    if (r < 0) {
        printf("Error in astr(). snprintf returned %i: r =  snprintf(buffer + used, buffer_size - used, \" ]\");\n", r);
        free(buffer);
        return NULL;
    }

    used += (size_t)r;

    if (used != buffer_size - 1) {
        printf("WARNING: snprintf returned %lu in astr() at end of method. used=%lu, (buffer_size-1) = %lu\n", used, used, (buffer_size -1));
    }

    return buffer;

}


// TESTING METHODS
// ----------------

#ifdef STR_UTILS_TEST_MAIN
void t_itostr(void) {
    char *c = itostr(42);  // must free

    printf("itostr(42) = %s\n", c);

    free(c);
}

void t_astr(void) {
    int nums[] = {1,2,3,4,5};
    char * result = astr( nums, 5);
    printf("astr( {1,2,3,4,5}, 5); returns : %s\n", result);
    free(result);

    const int empty[] = {};
    result = astr( empty, 0);
    printf("astr( {}, 0); returns : %s\n", result);
    free(result);

    result = astr( NULL, 0);
    printf("astr( NULL, 0); returns : %s\n", result);
    free(result);
}

void t_astr_dynamically(void) {

    const range r1 = new_range(1, 4, 1);

    const range r = r1;

    for (int i = r.start; i < r.stop; i+=r.step ) {
        // each i represents a power of 10
        const int n = (int)pow(10, i);
        int a[ n ];
        for (int j = 0; j < n; j++) {
            a[j] = j;
        }

        char *a_str = astr(a, (size_t)n);

        if (a_str != NULL) {
            printf("array n=%i : %s\n", n, a_str);
            free(a_str);
        }
    }
}

void t_concat() {
    char *s = concat("Hellow", " World!");
    printf("%s\n", s);
    free(s);

    char s1[50] = "Hello";
    char *s2 = "World";
    concat_strings( s1, s2);
    printf("dest = %s\n", s1);
}

int main(void) {
    // t_two_sum_decision();
    // t_itostr();
    // t_astr();
    // t_astr_dynamically();
    t_concat();

    void array[];

    return 0;
}

#endif