#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

typedef struct Vector {
    size_t length;  // current number of elements in the vector
    size_t capacity;  // total number of elements the buffer can hold
    int *v;
} vector_t;

typedef enum {
    VEC_OK = 0,
    VEC_ERR_NULL_ARG,
    VEC_ERR_ZERO_CAP,
    VEC_ERR_OVERFLOW,
    VEC_ERR_OUT_OF_MEM,
    VEC_ERR_EMPTY,
    VEC_ERR_INDEX_OUT_OF_RANGE
} vec_err_t;


/**
 * @param err error code returned by vector functions
 * @return human-readable string for the error code
 */
const char *vec_err_str(vec_err_t err);

/**
 * @param out destination vector to initialize (zeroed on error)
 * @param initial_capacity number of int elements to pre-allocate space for
 * @return VEC_OK on success, or an error code on failure (out is zeroed on error). Use vec_err_str to get a
 * human-readable error message.
 */
vec_err_t vector_init(vector_t *out, size_t initial_capacity);

/**
 * @param v vector to append to
 * @param i element to append
 * @return VEC_OK on success, or an error code on failure
 */
vec_err_t vector_append(vector_t *v, int i);

/**
 * @param v vector to pop from
 * @param out destination for the popped value
 * @return VEC_OK on success, or an error code on failure
 */
vec_err_t vector_pop(vector_t *v, int *out);

/**
 * @param v vector to pop from
 * @param fallback value to return when v is NULL or empty
 * @return the popped value, or fallback if no value was available
 */
int vector_pop_or(vector_t *v, int fallback);

/**
 * For internal use. Undefined on NULL/empty.
 * @param v vector to pop from
 * @return the popped value
 */
int vector_pop_unsafe(vector_t *v);

#endif // VECTOR_H
