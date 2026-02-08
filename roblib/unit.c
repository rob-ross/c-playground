//
// Created by Rob Ross on 2/6/26.
//

#include "unit.h"

#include <setjmp.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__SUNPRO_CC) || defined(__IBMCPP__)
#  define PUNIT_THREAD_LOCAL __thread
#elif (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201102L)) || defined(_Thread_local)
#  define PUNIT_THREAD_LOCAL _Thread_local
#endif

/*** Logging ***/

static PunitLogLevel punit_log_level_visible = PUNIT_LOG_INFO;
static PunitLogLevel punit_log_level_fatal = PUNIT_LOG_ERROR;

#if defined(PUNIT_THREAD_LOCAL)
static PUNIT_THREAD_LOCAL bool punit_error_jmp_buf_valid = false;
static PUNIT_THREAD_LOCAL jmp_buf punit_error_jmp_buf;
#endif



PUNIT_PRINTF(5,0)
static void
punit_logf_exv(PunitLogLevel level, FILE* fp, const char* filename, int line, const char* func, const char* format, va_list ap) {
  if (level < punit_log_level_visible)
    return;

  if (filename != NULL) {
    char abs_path[PATH_MAX];
    if (realpath(filename, abs_path) != NULL) {
      fprintf(fp, "%s:%d: ", abs_path, line);
    } else {
      fprintf(fp, "%s:%d: ", filename, line);
    }
  }

  if (func != NULL) {
      fprintf(fp, "%s: ", func);
  }

  switch (level) {
    case PUNIT_LOG_DEBUG:
      fputs("Debug", fp);
      break;
    case PUNIT_LOG_INFO:
      fputs("Info", fp);
      break;
    case PUNIT_LOG_WARNING:
      fputs("Warning", fp);
      break;
    case PUNIT_LOG_ERROR:
      fputs("Error", fp);
      break;
    default:
      punit_logf_ex(PUNIT_LOG_ERROR, filename, line, func,"Invalid log level (%d)", level);
      return;
  }

  fputs(": ", fp);
  vfprintf(fp, format, ap);
  fputc('\n', fp);
}

PUNIT_PRINTF(3,4)
static void
punit_logf_internal(PunitLogLevel level, FILE* fp, const char* format, ...) {
  va_list ap;

  va_start(ap, format);
  punit_logf_exv(level, fp, NULL, 0, NULL, format, ap);
  va_end(ap);
}

static void
punit_log_internal(PunitLogLevel level, FILE* fp, const char* message) {
  punit_logf_internal(level, fp, "%s", message);
}

// the "_ex" stands for extended, as in "extended interface"
void
punit_logf_ex(PunitLogLevel level, const char* filename, int line, const char* func, const char* format, ...) {
  va_list ap;

  va_start(ap, format);
  punit_logf_exv(level, stderr, filename, line, func, format, ap);
  va_end(ap);

  if (level >= punit_log_level_fatal) {
#if defined(PUNIT_THREAD_LOCAL)
    if (punit_error_jmp_buf_valid) {
      longjmp(punit_error_jmp_buf, 1);
    }
#endif
    abort();
  }
}

void
punit_errorf_ex(const char* filename, int line, const char* func, const char* format, ...) {
  va_list ap;

  va_start(ap, format);
  punit_logf_exv(PUNIT_LOG_ERROR, stderr, filename, line, func, format, ap);
  va_end(ap);

#if defined(PUNIT_THREAD_LOCAL)
  if (punit_error_jmp_buf_valid) {
    longjmp(punit_error_jmp_buf, 1);
  }
#endif
  abort();
}


// ---------------------------------------------------

// each variadic arg is expected to be a string representing a conversion specifier, like "%i".
// buffer needs to be large enough for the resulting string
char * insert_conversion_specifiers(const char *format_str, ...) {
  static PUNIT_THREAD_LOCAL char buffer[1024];
  va_list args;
  va_start(args, format_str);
  vsnprintf(buffer, sizeof(buffer), format_str, args);
  va_end(args);
  return buffer;
}






// test runner experiments
int tests_run = 0;
int tests_failed = 0;

void run_test(test_case test) {
  // emulate try-catch with longjmp
  if (setjmp(punit_error_jmp_buf) == 0) {
    //Try block
    punit_error_jmp_buf_valid = true;
    tests_run++;
    test();  // invoke test method
  } else {
    //Except block
    tests_failed++;
  }
  punit_error_jmp_buf_valid = false;
}

void test_runner(test_case tests[]) {
  size_t index = 0;
  test_case t = tests[0];
  while ( t != NULL) {
    run_test(t);
    t = tests[++index];
  }
  printf("Test runner: tests run: %i, tests failed: %i\n", tests_run, tests_failed);
}