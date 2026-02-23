#pragma once

#include <stddef.h>


// element type is either scalar/primative or ptr type.

typedef struct Vector {
    size_t length;  // current number of elements in the vector
    size_t capacity;  // total number of elements the buffer can hold
    char* type_name; // eventually this will be an enum, we have identified 46 types so far.
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

/** Add the value `i` to the end of the Vector, increasing its length by 1.
 * @param v vector to append to
 * @param i element to append
 * @return VEC_OK on success, or an error code on failure
 */
vec_err_t vector_append(vector_t *v, int i);


/**
 * Insert the value `i` in the Vector at index position `index`. This causes the value of Vector[index] to contain
 * the new value `i`, increases the length of the Vector by 1, and moves every existing value from the
 * original Vector[index] to the right by 1 place.
 * @param v vector to insert into
 * @param i element to insert
 * @param index the index position in the vector at which to insert `i`, from 0 to length-1
 * @return VEC_OK on success, or an error code on failure
 */
vec_err_t vector_insert(vector_t *v, int i, size_t index);

/**
 * Returns the value at index
 * @param v vector to retreive value from
 * @param out destination for the returned value
 * @param index the index position in the vector, from 0 to length-1
 * @return the value at the index given by the `index` argument. If `index` is invalid returns
 * VEC_ERR_INDEX_OUT_OF_RANGE
 */
vec_err_t vector_get(vector_t *v, int *out, size_t index);



/**
 * Returns in `out` the value at index or the fallback value if index is out of range of 0 to length-1.
 * @param v vector to retreive value from
 * @param out destination for the returned value
 * @param index the index position in the vector, from 0 to length-1
 * @param fallback if index is not in range of 0 to length-1, returns this value instead.
 * @return the value at the index given by the `index` argument, or `fallback` if index is out of range.
 */
vec_err_t vector_get_or(vector_t *v, int *out, size_t index, int fallback);



/**
 * Removes the last element and returns it in `out`. After this call, the length of the vector is reduced by 1.
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


