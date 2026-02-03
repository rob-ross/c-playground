//
// Created by Rob Ross on 1/28/26.
//
// Implementation of a dynamically sized array that can add new elements, select elements, and grow as needed.
// to make: clang -DVECTOR_TEST_MAIN vector.c -o vector

#include <stdlib.h>

#include "vector.h"

#include <stdio.h>
#include <string.h>

const char *vec_err_str(const vec_err_t err)
{
    switch (err) {
        case VEC_OK:
            return "ok";
        case VEC_ERR_NULL_ARG:
            return "null argument";
        case VEC_ERR_ZERO_CAP:
            return "zero capacity";
        case VEC_ERR_OVERFLOW:
            return "size overflow";
        case VEC_ERR_OUT_OF_MEM:
            return "out of memory";
        case VEC_ERR_EMPTY:
            return "empty vector";
        case VEC_ERR_INDEX_OUT_OF_RANGE:
            return "index out of range";
        default:
            return "unknown error";
    }
}

vec_err_t vector_init (vector_t *out, const size_t initial_capacity) {
    if (!out) {
        return VEC_ERR_NULL_ARG;
    }
    *out = (vector_t){0};
    if (initial_capacity == 0 ) {
        return VEC_ERR_ZERO_CAP;
    }
    if ( initial_capacity > SIZE_MAX / sizeof(int)) {
        return VEC_ERR_OVERFLOW; // overflow guard
    }

    out->v = calloc(initial_capacity, sizeof(int));
    if ( !out->v) {
        return VEC_ERR_OUT_OF_MEM;
    }
    out->capacity = initial_capacity;
    return VEC_OK;
}

vec_err_t _ensure_capacity(vector_t *v) {
    if (v->length >= v->capacity) {
        // must grow buffer
        size_t new_capacity = v->capacity ? v->capacity * 2 : 1;
        if (v->capacity > SIZE_MAX / 2 || new_capacity > SIZE_MAX / sizeof(int)) {
            return VEC_ERR_OVERFLOW;
        }
        int *re_ptr = realloc(v->v, new_capacity * sizeof(int));
        if (!re_ptr) {
            return VEC_ERR_OUT_OF_MEM;
        }
        v->v = re_ptr;
        v->capacity = new_capacity;
    }
    return VEC_OK;
}

vec_err_t vector_append(vector_t *v, const int i) {
    if (!v) {
        return VEC_ERR_NULL_ARG;
    }
    const int result = _ensure_capacity(v);
    if (result  != VEC_OK) {
        return result;
    }
    v->v[v->length++] = i;
    return VEC_OK;
}

//insert i at index position. All subsequent elements are shifted down by one.
vec_err_t vector_insert(vector_t *v, const int i, const size_t index) {
    if (!v) {
        return VEC_ERR_NULL_ARG;
    }
    const int result = _ensure_capacity(v);
    if (result  != VEC_OK) {
        return result;
    }
    v->length++;
    for (size_t j = index+1; j < v->length; ++j) {
        v->v[j] = v->v[j-1];
    }
    v->v[index] = i;

    return VEC_OK;
}

vec_err_t vector_pop(vector_t *v, int *out) {
    if (!v || !out) {
        return VEC_ERR_NULL_ARG;
    }
    if (v->length == 0 ) {
        return VEC_ERR_EMPTY;
    }
    *out = v->v[v->length - 1];
    v->length--;
    return VEC_OK;
}

int vector_pop_or(vector_t *v, int fallback ) {
    if (!v || v->length == 0 ) {
        return fallback;
    }
    v->length--;
    return v->v[v->length];
}

int vector_pop_unsafe(vector_t *v) {
    v->length--;
    return v->v[v->length];
}

/**
 * Remove the element at `index` and return the value in `out`. Values > index move down by one to fill the hole.
 * @param v
 * @param index
 * @param out
 * @return
 */
vec_err_t vector_remove(vector_t *v, const size_t index, int *out) {
    if (!v || !out) {
        return VEC_ERR_NULL_ARG;
    }
    if (v->length == 0 ) {
        return VEC_ERR_EMPTY;
    }
    if (  v->length - 1 < index ) {
        return VEC_ERR_INDEX_OUT_OF_RANGE;
    }
    *out = v->v[index];
    // copy v[i + 1] to v[i] for all i > index
    if (index + 1 < v->length) {
        memmove(&v->v[index], &v->v[index + 1],
                (v->length - index - 1) * sizeof v->v[0]);
    }
    v->length--;
    return VEC_OK;
}



#ifdef VECTOR_TEST_MAIN

void display_vector(vector_t v) {
    printf("vector length=%zu, capacity=%zu [", v.length, v.capacity);
    for (int i = 0; i < v.length; i++ ) {
        printf("%i,",v.v[i]);
    }
    printf("]\n");
}

int main(void) {
    // quick test of vector
    vector_t vec;
    vec_err_t err = vector_init(&vec, 2);

    for (int i = 0; i < 10; i++) {
        printf("err=%s ", vec_err_str(err));
        display_vector(vec);
        err = vector_append(&vec, i);
    }
    printf("err=%s ", vec_err_str(err));
    display_vector(vec);
    printf("\nRemoving;");

    for (int i = 0; i < 10; i++) {
        printf("err=%s ", vec_err_str(err));
        display_vector(vec);
        int out;
        err = vector_remove(&vec, 1,  &out);  // removing first element each time
    }
    printf("err=%s ", vec_err_str(err));
    display_vector(vec);

}

#endif
