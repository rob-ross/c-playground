//
// Created by Rob Ross on 1/29/26.
//


#include <stdio.h>
#include <string.h>

#include "munit/munit.h"
#include "munit/munit_overrides.h"

// make : clang concat.c munit/munit.c -o concat

typedef struct {
    size_t length;
    char buffer[64];
} TextBuffer;

size_t min(size_t int1, size_t int2) {
    if (int1 < int2) {
        return int1;
    }
    return int2;
}

typedef char* string;

int smart_append(TextBuffer *dest, const char *src) {
    if (!dest || !src || dest->length >= 63) {
        return 1;
    }
    size_t src_len = strlen(src);
    size_t space_available = 63 - ( dest->length  ) ;
    size_t num_chars_to_copy = min(space_available, src_len);

    // printf("\nsrc='%s', src_len=%zu, buffer='%s', buffer.length=%zu, avail=%zu, num_to_copy=%zu\n", src, src_len, dest->buffer, dest->length, space_available, num_chars_to_copy);

    strncat(dest->buffer, src, num_chars_to_copy);
    dest->length += num_chars_to_copy;
    dest->buffer[dest->length] = '\0';

    return src_len > space_available;
}


munit_case(RUN, test_return_1_for_null_value, {
  TextBuffer dest;
  const char *src = NULL;
  int result = smart_append(&dest, src);
  munit_assert_int(result, ==, 1, "Should return 1 for null value");
});

munit_case(RUN, test_smart_append_empty_buffer, {
  TextBuffer dest;
  strcpy(dest.buffer, "");
  dest.length = 0;
  const char *src = "Hello";
  int result = smart_append(&dest, src);
  munit_assert_int(result, ==, 0, "Should return 0 for successful append");
  munit_assert_string_equal(dest.buffer, "Hello",
                            "Buffer should contain 'Hello'");
  munit_assert_int(dest.length, ==, 5, "Length should be 5");
});

munit_case(SUBMIT, test_smart_append_full_buffer, {
  TextBuffer dest;
  strcpy(dest.buffer,
         "This is a very long string that will fill up the entire buffer.");
  dest.length = 63;
  const char *src = " Extra";
  int result = smart_append(&dest, src);
  munit_assert_int(result, ==, 1, "Should return 1 for unsuccessful append");
  munit_assert_string_equal(
      dest.buffer,
      "This is a very long string that will fill up the entire buffer.",
      "Buffer should remain unchanged");
  munit_assert_int(dest.length, ==, 63, "Length should remain 63");
});

munit_case(SUBMIT, test_smart_append_overflow, {
  TextBuffer dest;
  strcpy(dest.buffer, "This is a long string");
  dest.length = 21;
  const char *src = " that will fill the whole buffer and leave no space for "
                    "some of the chars.";
  int result = smart_append(&dest, src);
  munit_assert_int(result, ==, 1, "Should return 1 for overflow append");
  munit_assert_string_equal(
      dest.buffer,
      "This is a long string that will fill the whole buffer and leave",
      "Buffer should be filled to capacity");
  munit_assert_int(dest.length, ==, 63,
                   "Length should be 63 after overflow append");
});


int boot_dev_runner(void) {
    MunitTest tests[]  = {
            munit_test("/test_return_1_for_null_value", test_return_1_for_null_value),
            munit_test("/test_smart_append_empty_buffer",
                       test_smart_append_empty_buffer),
            munit_test("/test_smart_append_full_buffer",
                       test_smart_append_full_buffer),
            munit_test("/test_smart_append_overflow", test_smart_append_overflow),
            munit_null_test,
        };

    MunitSuite suite = munit_suite("smart_append", tests);

    return munit_suite_main(&suite, NULL, 0, NULL);
}



int main(void) {
    int result = boot_dev_runner();

    return result;
}