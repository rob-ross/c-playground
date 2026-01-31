

#include <stdlib.h>
#include <stdio.h>
#include "./munit/munit.h"
#include "./munit/munit_overrides.h"

// make: clang pointer_pointer.c ./munit/munit.c -o pointer_pointer.out

void allocate_int(int **pointer_pointer, int value);


void allocate_int(int **pointer_pointer, int value) {
    int *int_p = malloc(sizeof(int));
    if (!int_p) {
        printf("Malloc failed.\n");
        exit(1);
    }
    *pointer_pointer = int_p;
    **pointer_pointer = value;
}

munit_case(RUN, test_allocate, {
  int *pointer = NULL;
  allocate_int(&pointer, 10);

  munit_assert_ptr_not_null(pointer, "Should allocate pointer");
  munit_assert_int(*pointer, ==, 10, "Should assign value to pointer");

  free(pointer);
});

munit_case(SUBMIT, test_does_not_overwrite, {
  int value = 5;
  int *pointer = &value;

  allocate_int(&pointer, 20);

  munit_assert_int(value, ==, 5, "Should not overwrite original value");

  munit_assert_ptr_not_null(pointer, "Should allocate pointer");
  munit_assert_int(*pointer, ==, 20, "Should assign value to pointer");

  free(pointer);
});

int main() {
    MunitTest tests[] = {
        munit_test("/create", test_allocate),
        munit_test("/overwrite", test_does_not_overwrite),
        munit_null_test,
    };
    MunitSuite suite = munit_suite("allocate_list", tests);

    return munit_suite_main(&suite, NULL, 0, NULL);
}
//
// Created by Rob Ross on 1/30/26.
//