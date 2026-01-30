//
// Created by Rob Ross on 1/29/26.
//



#include "munit/munit.h"
#include "munit/munit_overrides.h"

//make : clang color_enum.c munit/munit.c -o color_enum.out


typedef enum Color {RED, GREEN, BLUE} color_t;

munit_case(RUN, test_color_enum1, {
  munit_assert_int(RED, ==, 0, "RED is defined as 0");
  munit_assert_int(GREEN, ==, 1, "GREEN is defined as 1");
  munit_assert_int(BLUE, ==, 2, "BLUE is defined as 2");
});

munit_case(SUBMIT, test_color_enum2, {
  munit_assert_int(RED, !=, 4, "RED is not defined as 4");
  munit_assert_int(GREEN, !=, 2, "GREEN is not defined as 2");
  munit_assert_int(BLUE, !=, 0, "BLUE is not defined as 0");
});

int main() {
    MunitTest tests[] = {
        munit_test("/are_defined", test_color_enum1),
        munit_test("/are_defined_correctly", test_color_enum2),
        munit_null_test,
    };

    MunitSuite suite = munit_suite("colors", tests);

    return munit_suite_main(&suite, NULL, 0, NULL);
}