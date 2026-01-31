
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "./munit/munit.h"
#include "./munit/munit_overrides.h"

// make: clang swap_integers.c ./munit/munit.c -o swap_integers.out

void swap_ints(int *a, int *b);

void swap_ints(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}


munit_case(RUN, test_swap_ints, {
  int a = 5;
  int b = 6;

  swap_ints(&a, &b);

  munit_assert_int(a, ==, 6, "a is now 6");
  munit_assert_int(b, ==, 5, "b is now 5");
});

munit_case(SUBMIT, test_swap_ints_same, {
  int a = 5;

  swap_ints(&a, &a);

  munit_assert_int(a, ==, 5, "a is still 5");
});

int main() {
    MunitTest tests[] = {
        munit_test("/swap_ints", test_swap_ints),
        munit_test("/swap_ints_same", test_swap_ints_same),
        munit_null_test,
    };

    MunitSuite suite = munit_suite("void-pointer", tests);
    return munit_suite_main(&suite, NULL, 0, NULL);
}
