
#include <assert.h>

#include "snekobject.h"
#include "../munit/munit.h"
#include "../munit/munit_overrides.h"
#include "../munit/memory_check.h"


// make clang length.c ../munit/munit.c ../munit/memory_check.c snekobject.c -o length.out

munit_case(RUN, test_integer, {
  snek_object_t *obj = new_snek_integer(42);
  munit_assert_int(snek_length(obj), ==, 1, "Integers are length 1");

  free(obj);
  assert(boot_all_freed());
});

munit_case(RUN, test_float, {
  snek_object_t *obj = new_snek_float(3.14);
  munit_assert_int(snek_length(obj), ==, 1, "Floats are length 1");

  free(obj);
  assert(boot_all_freed());
});

munit_case(RUN, test_string, {
  snek_object_t *shorter = new_snek_string("hello");
  munit_assert_int(snek_length(shorter), ==, 5, "Should return based on input");

  snek_object_t *longer = new_snek_string("hello, world");
  munit_assert_int(snek_length(longer), ==, strlen("hello, world"),
             "Should use strlen");

  free(shorter->data.v_string);
  free(shorter);
  free(longer->data.v_string);
  free(longer);
  assert(boot_all_freed());
});

munit_case(SUBMIT, test_vector3, {
  snek_object_t *i = new_snek_integer(1);
  snek_object_t *vec = new_snek_vector3(i, i, i);
  munit_assert_int(snek_length(vec), ==, 3, "Vec3 always has length 3");

  free(i);
  free(vec);
  assert(boot_all_freed());
});

munit_case(SUBMIT, test_array, {
  snek_object_t *i = new_snek_integer(1);
  snek_object_t *arr = new_snek_array(4);
  assert(snek_array_set(arr, 0, i));
  assert(snek_array_set(arr, 1, i));
  assert(snek_array_set(arr, 2, i));

  munit_assert_int(snek_length(arr), ==, 4, "Length of array should be its size");

  free(i);
  free(arr->data.v_array.elements);
  free(arr);
  assert(boot_all_freed());
});

int main() {
    MunitTest tests[] = {
        munit_test("/integer", test_integer), munit_test("/float", test_float),
        munit_test("/string", test_string),   munit_test("/vector", test_vector3),
        munit_test("/array", test_array),     munit_null_test,
    };

    MunitSuite suite = munit_suite("object-length", tests);

    return munit_suite_main(&suite, NULL, 0, NULL);
}
