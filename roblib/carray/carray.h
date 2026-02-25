//
// Created by Rob Ross on 2/24/26.
//

#pragma once
#include <stddef.h>



typedef enum CArrayError {
    CARRAY_OK = 0,
    CARRAY_ERR_NULL_ARG,
    CARRAY_ERR_OUT_OF_MEM,
    CARRAY_ERR_EMPTY,
    CARRAY_ERR_INDEX_OUT_OF_RANGE
} CArrayError;

/**
 * @param err error code returned by Array functions
 * @return human-readable string for the error code
 */
const char *carray_err_str(CArrayError err);

/**
 * Returns the element of `array` at `index` in the `out` parameter. Returns CARRAY_OK on success, or an error value
 * in case of failure. If the method does not return CARRAY_OK, -1 is returned in the `out` parameter.
 * @param n number of elements in `array`
 * @param array the array from which to get the element
 * @param index the index position of the array. If greater than n-1, returns CARRAY_ERR_INDEX_OUT_OF_RANGE
 * @param out either the value at array[index], or -1 if an error occurrs.
 * @return the value at element array[index] in the `out` parameter or -1. This method returns CARRAY_OK on success
 * and an error code on failure.
 */
CArrayError carray_get_int(size_t n, const int array[static n], size_t index, int *out);

/**
 * Prints a representation of the array via printf, on the current line. Does not print a newline.
 * If `limit` is greater than 0, uses this to limit the number of displayed elements to MIN(n, limit).
 * If `message` is not null, uses this string as the output prefix instead of array element type.
 * @param n number of elements in the array
 * @param array the array to display
 * @param message optional, if null prefixes output with array element type, otherwise uses this as the prefix
 * @param limit if > 0, restricts output to the first MIN(n, limit) array elements. If 0, limits to DEFAULT_LIMIT items.
 */
void carray_repr_int(size_t n, const int array[static n], char const *message, size_t limit);