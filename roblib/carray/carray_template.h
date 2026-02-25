//
//  carray_template.h
//
// Created by Rob Ross on 2/24/26.
//

#pragma once

#include "../base.h"


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

#define DEFAULT_LIMIT 10

// -------------------------------------
// Helper Macros
// -------------------------------------

#define CARRAY_FN(name) CAT3(carray, name, CARRAY_NAME)


