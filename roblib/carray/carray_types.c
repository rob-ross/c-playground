//
//  carray_types.c
//
// Created by Rob Ross on 2/24/26.
//

#include "carray_template.h"

const char *carray_err_str(const CArrayError err) {
    switch (err) {
        case CARRAY_OK:
            return "ok";
        case CARRAY_ERR_NULL_ARG:
            return "null argument";
        case CARRAY_ERR_OUT_OF_MEM:
            return "out of memory";
        case CARRAY_ERR_EMPTY:
            return "empty array";
        case CARRAY_ERR_INDEX_OUT_OF_RANGE:
            return "index out of range";
        default:
            return "unknown error";
    }

}

#if !defined(CARRAY_IMPLEMENTATION)
#   define CARRAY_IMPLEMENTATION
#   include "carray_types.h"
#endif