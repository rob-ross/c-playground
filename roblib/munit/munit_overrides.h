//
// Created by Rob Ross on 1/29/26.
//
// overrides to standard munit macros to work with bootdev test runner.


#ifndef CS50X_MUNIT_OVERRIDES_H
#define CS50X_MUNIT_OVERRIDES_H

#include "munit.h"


#ifdef munit_assert_float
#undef munit_assert_float
#define munit_assert_float(a, op, b, c) \
    munit_assert_type(float, "f", a, op, b)
#endif

#ifdef munit_assert_int
#undef munit_assert_int
// added third parameter, unused by macro
#define munit_assert_int(a, op, b, c) \
    munit_assert_type(int, "d", a, op, b)
#endif


#define munit_assert_int_equal(a, b, c) \
    munit_assert_int(a, ==, b, c)

#ifdef munit_assert_string_equal
#undef munit_assert_string_equal
#endif

// added third parameter, unused by macro
#define munit_assert_string_equal(a, b, c) \
    do { \
        const char* munit_tmp_a_ = a; \
        const char* munit_tmp_b_ = b; \
        if (MUNIT_UNLIKELY(strcmp(munit_tmp_a_, munit_tmp_b_) != 0)) { \
        munit_errorf("assertion failed: string %s == %s (\"%s\" == \"%s\")", \
        #a, #b, munit_tmp_a_, munit_tmp_b_); \
        } \
        MUNIT_PUSH_DISABLE_MSVC_C4127_ \
    } while (0) \
    MUNIT_POP_DISABLE_MSVC_C4127_

#ifdef munit_assert_ptr_equal
#undef munit_assert_ptr_equal
#define munit_assert_ptr_equal(a, b, c) \
    munit_assert_ptr(a, ==, b)
#endif

#ifdef munit_assert_ptr_not_equal
#undef munit_assert_ptr_not_equal
#define munit_assert_ptr_not_equal(a, b, c) \
    munit_assert_ptr(a, !=, b)
#endif

#ifdef munit_assert_ptr_not_null
#undef munit_assert_ptr_not_null
#define munit_assert_ptr_not_null(ptr, a) \
    munit_assert_ptr(ptr, !=, NULL)
#endif

#ifdef munit_assert_ptr_null
#undef munit_assert_ptr_null
#define munit_assert_ptr_null(ptr, c) \
    munit_assert_ptr(ptr, ==, NULL)
#endif

#ifdef munit_assert_null
#undef munit_assert_null
#define munit_assert_null(ptr, a) munit_assert_ptr(ptr, ==, NULL)
#endif

#ifdef munit_assert_not_null
#undef munit_assert_not_null
#define munit_assert_not_null(ptr, a) munit_assert_ptr(ptr, !=, NULL)

#endif

#ifdef munit_assert_size
#undef munit_assert_size
#define munit_assert_size(a, op, b, c) \
    munit_assert_type(size_t, MUNIT_SIZE_MODIFIER "u", a, op, b)
#endif

#ifdef munit_assert_uint8
#undef munit_assert_uint8
#define munit_assert_uint8(a, op, b, c) \
    munit_assert_type(munit_uint8_t, PRIu8, a, op, b)
#endif

#ifdef munit_assert_uint16
#undef munit_assert_uint16
#define munit_assert_uint16(a, op, b, c) \
    munit_assert_type(munit_uint16_t, PRIu16, a, op, b)
#endif

#ifdef munit_assert_uint32
#undef munit_assert_uint32
#define munit_assert_uint32(a, op, b, c) \
    munit_assert_type(munit_uint32_t, PRIu32, a, op, b)
#endif



#define munit_case(a, b, ...) \
MunitResult b(const MunitParameter params[], void* user_data_or_fixture) {\
    do {                \
       __VA_ARGS__      \
    } while (0);        \
    return MUNIT_OK;    \
}

#define munit_test(a, b) \
(MunitTest){ .name=a, .test=b, .setup=NULL, .tear_down=NULL, .options=MUNIT_TEST_OPTION_NONE, .parameters=NULL }

#define munit_null_test { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }

#define munit_suite( a, b) (MunitSuite){ .prefix=a, .tests=b, .suites=NULL, .iterations=1, .options=MUNIT_SUITE_OPTION_NONE }


#endif //CS50X_MUNIT_OVERRIDES_H