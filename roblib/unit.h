//
// Created by Rob Ross on 2/6/26.
//

#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <inttypes.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#  define PUNIT_NO_RETURN _Noreturn
#elif defined(__GNUC__)
#  define PUNIT_NO_RETURN __attribute__((__noreturn__))
#else
#  define PUNIT_NO_RETURN
#endif

typedef enum {
    PUNIT_LOG_DEBUG,
    PUNIT_LOG_INFO,
    PUNIT_LOG_WARNING,
    PUNIT_LOG_ERROR
  } PunitLogLevel;

#if defined(__GNUC__) && !defined(__MINGW32__)
#  define PUNIT_PRINTF(string_index, first_to_check) __attribute__((format (printf, string_index, first_to_check)))
#else
#  define PUNIT_PRINTF(string_index, first_to_check)
#endif

PUNIT_PRINTF(5, 6)
void punit_logf_ex(PunitLogLevel level, const char* filename, int line, const char* func, const char* format, ...);

#define punit_logf(level, format, ...) \
punit_logf_ex(level, __FILE__, __LINE__, __func__, format, __VA_ARGS__)

#define punit_log(level, msg) \
punit_logf(level, "%s", msg)

PUNIT_NO_RETURN
PUNIT_PRINTF(4, 5)
void punit_errorf_ex(const char* filename, int line, const char* func, const char* format, ...);

#define punit_errorf(format, ...) \
    punit_errorf_ex(__FILE__, __LINE__, __func__, format, __VA_ARGS__)

#define punit_error(msg) \
    punit_errorf("%s", msg)


// -----------
// assertTrue
// -----------


#define assertTrue_1(expr) \
do { \
    if (!(expr)) { \
        punit_error("assertion failed: " #expr " is not true."); \
    } \
} while (0)

#define assertTrue_2(expr, msg) \
do { \
    if (!(expr)) { \
        punit_errorf("assertion failed: " #expr " is not true. %s", (msg)); \
    } \
} while (0)

#define assertTrue_SELECT(_1, _2, NAME, ...) NAME
#define assertTrue(...) assertTrue_SELECT(__VA_ARGS__, assertTrue_2, assertTrue_1)(__VA_ARGS__)

// -----------
// assertFalse
// -----------

// ------------------------
// assertEqual
// ------------------------
#define PUNIT_ASSERT_EQUAL_BODY(a, b, fail_call) \
    do { \
        if (!( a == b )) { \
            fail_call; \
        } \
    } while (0) \

// Helper to select format specifier based on type (C11)
#define PUNIT_FMT(expr, ...) _Generic( (expr), \
    char              : "%c",   \
    unsigned char     : "%hhu", \
    signed char       : "%hhi", \
    short             : "%hi",  \
    unsigned short    : "%hu",  \
    int               : "%i",   \
    unsigned int      : "%u",   \
    long              : "%li",  \
    unsigned long     : "%lu",  \
    long long         : "%lli", \
    unsigned long long: "%llu", \
    float             : "%f",   \
    double            : "%f",   \
    long double       : "%Lf"   \
)

#define punit_fmt_error_msg(msg, ...) \



#define punit_assertEqual_1(a, b) \
PUNIT_ASSERT_EQUAL_BODY((a), (b), \
    punit_errorf( \
    insert_conversion_specifiers("assertion failed.\nExpected: %s \nActual  : %s", \
        PUNIT_FMT(a), PUNIT_FMT(b) ), (a), (b)))

#define punit_assertEqual_2(a, b, msg) \
PUNIT_ASSERT_EQUAL_BODY((a), (b), \
    punit_errorf(\
    insert_conversion_specifiers("assertion failed: %s\nExpected: %s\nActual  : %s", \
        "%s", PUNIT_FMT(a), PUNIT_FMT(b) ), (msg), (a), (b)))

#define punit_assertEqual_SELECT(_1, _2, _3, NAME, ...) NAME
#define assertEqual(...) \
    punit_assertEqual_SELECT(__VA_ARGS__, punit_assertEqual_2, punit_assertEqual_1)(__VA_ARGS__)

// -------------------------------------------------------
// tests
char *insert_conversion_specifiers(const char *format_str, ...);

typedef void (*test_case)(void);
void run_test(test_case test);
void test_runner(test_case tests[]);