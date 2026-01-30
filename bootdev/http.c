//
// Created by Rob Ross on 1/29/26.
//

#include "munit/munit.h"
#include "munit/munit_overrides.h"

//make : clang http.c munit/munit.c -o http.out


typedef enum {
    HTTP_BAD_REQUEST = 400,
    HTTP_UNAUTHORIZED = 401,
    HTTP_NOT_FOUND = 404,
    HTTP_TEAPOT = 418,
    HTTP_INTERNAL_SERVER_ERROR = 500
  } http_error_code_t;

char *http_to_str(http_error_code_t code);


char *http_to_str(http_error_code_t code) {
    switch (code) {
        case HTTP_BAD_REQUEST:
            return  "400 Bad Request";
        case HTTP_UNAUTHORIZED:
            return "401 Unauthorized";
        case HTTP_NOT_FOUND:
            return "404 Not Found";
        case HTTP_TEAPOT:
            return "418 I AM A TEAPOT!";
        case HTTP_INTERNAL_SERVER_ERROR:
            return "500 Internal Server Error";
        default:
            return "Unknown HTTP status code";
    }
}



munit_case(RUN, test_switch_enum, {
  munit_assert_string_equal(http_to_str(HTTP_BAD_REQUEST), "400 Bad Request", "");
  munit_assert_string_equal(http_to_str(HTTP_UNAUTHORIZED), "401 Unauthorized", "");
  munit_assert_string_equal(http_to_str(HTTP_NOT_FOUND), "404 Not Found", "");
  munit_assert_string_equal(http_to_str(HTTP_TEAPOT), "418 I AM A TEAPOT!", "");
  munit_assert_string_equal(http_to_str(HTTP_INTERNAL_SERVER_ERROR),
                      "500 Internal Server Error", "");
});

munit_case(SUBMIT, test_switch_enum_default, {
  munit_assert_string_equal(http_to_str((http_error_code_t)999),
                      "Unknown HTTP status code", "");
});

int main() {
    MunitTest tests[] = {
        munit_test("/switch_enum", test_switch_enum),
        munit_test("/switch_enum_default", test_switch_enum_default),
        munit_null_test,
    };

    MunitSuite suite = munit_suite("http", tests);

    return munit_suite_main(&suite, NULL, 0, NULL);
}