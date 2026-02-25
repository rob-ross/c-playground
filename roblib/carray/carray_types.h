//
//  carray_types.h
//
// Created by Rob Ross on 2/24/26.
//

#pragma once

#define CARRAY_T bool
#define CARRAY_NAME _bool
#include "carray_template.inc"
#undef CARRAY_T
#undef CARRAY_NAME

#define CARRAY_T bool*
#define CARRAY_NAME _bool_ptr
#include "carray_template.inc"
#undef CARRAY_T
#undef CARRAY_NAME

#define CARRAY_T int
#define CARRAY_NAME _int
#include "carray_template.inc"
#undef CARRAY_T
#undef CARRAY_NAME

// #define CARRAY_T const int
// #define CARRAY_NAME _int_const
// #include "carray_template.inc"
// #undef CARRAY_T
// #undef CARRAY_NAME

#define CARRAY_T double
#define CARRAY_NAME _double
#include "carray_template.inc"
#undef CARRAY_T
#undef CARRAY_NAME

#define ELM_TYPE( A ) typeof( (A)[0] )
#define repr( N, A, M, L) _Generic((A)[0],  \
    bool: carray_repr_bool,                 \
    bool *: carray_repr_bool_ptr,           \
    bool const *: carray_repr_bool_ptr,     \
    int: carray_repr_int,                   \
    double: carray_repr_double              \
)(N, A, M, L)