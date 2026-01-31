
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "./munit/munit.h"
#include "./munit/munit_overrides.h"

// make: clang swap_strings.c ./munit/munit.c -o swap_strings.out

void swap_strings(char **a, char **b);

void swap_strings(char **a, char **b) {
    char *temp = *a;
    *a = *b;
    *b = temp;
}


munit_case(RUN, test_swap_str, {
  char *a = "Hello";
  char *b = "Goodbye";

  swap_strings(&a, &b);

  munit_assert_string_equal(a, "Goodbye", "a is now 'Goodbye'");
  munit_assert_string_equal(b, "Hello", "b is now 'Hello'");
});

munit_case(SUBMIT, test_swap_str_long, {
  char *a = "terminal.shop";
  char *b = "ssh";

  swap_strings(&a, &b);

  munit_assert_string_equal(a, "ssh", "a is now 'ssh'");
  munit_assert_string_equal(b, "terminal.shop", "b is now 'terminal.shop'");
});

int main() {
    MunitTest tests[] = {
        munit_test("/swap_str", test_swap_str),
        munit_test("/test_swap_str_long", test_swap_str_long),
        munit_null_test,
    };

    MunitSuite suite = munit_suite("void-pointer", tests);
    return munit_suite_main(&suite, NULL, 0, NULL);
}
