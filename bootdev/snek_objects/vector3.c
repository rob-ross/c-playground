

#include <assert.h>
#include <stdlib.h>

#include "snekobject.h"
#include "../munit/munit.h"
#include "../munit/munit_overrides.h"
#include "../munit/memory_check.h"

// make clang vector3.c ../munit/munit.c ../munit/memory_check.c snekobject.c -o vector3.out

munit_case(RUN, test_returns_null, {
  snek_object_t *vec = new_snek_vector3(NULL, NULL, NULL);

  munit_assert_null(vec, "Should return null when input is null");

  assert(boot_all_freed());
});

munit_case(RUN, test_vec_multiple_objects, {
  snek_object_t *x = new_snek_integer(1);
  snek_object_t *y = new_snek_integer(2);
  snek_object_t *z = new_snek_integer(3);
  snek_object_t *vec = new_snek_vector3(x, y, z);

  munit_assert_ptr_not_null(vec, "should allocate a new object");

  // Vectors should not copy objects, they get the reference to the objects.
  munit_assert_ptr_equal(x, vec->data.v_vector3.x, "should reference x");
  munit_assert_ptr_equal(y, vec->data.v_vector3.y, "should reference y");
  munit_assert_ptr_equal(z, vec->data.v_vector3.z, "should reference z");

  // Assert we have integer values correct
  munit_assert_int(vec->data.v_vector3.x->data.v_int, ==, 1, "should have correct x");
  munit_assert_int(vec->data.v_vector3.y->data.v_int, ==, 2, "should have correct y");
  munit_assert_int(vec->data.v_vector3.z->data.v_int, ==, 3, "should have correct z");

  // Free all of our objects.
  free(x);
  free(y);
  free(z);
  free(vec);
  stats();
  assert(boot_all_freed());
});

munit_case(SUBMIT, test_vec_same_object, {
  snek_object_t *i = new_snek_integer(1);
  snek_object_t *vec = new_snek_vector3(i, i, i);

  munit_assert_ptr_not_null(vec, "should allocate a new object");

  // Vectors should not copy objects, they get the reference to the objects.
  munit_assert_ptr_equal(i, vec->data.v_vector3.x, "should reference x");
  munit_assert_ptr_equal(i, vec->data.v_vector3.y, "should reference y");
  munit_assert_ptr_equal(i, vec->data.v_vector3.z, "should reference z");

  // Assert we have integer values correct
  munit_assert_int(vec->data.v_vector3.x->data.v_int, ==, 1, "should have correct x");
  munit_assert_int(vec->data.v_vector3.y->data.v_int, ==, 1, "should have correct y");
  munit_assert_int(vec->data.v_vector3.z->data.v_int, ==, 1, "should have correct z");

  i->data.v_int = 2;

  // Assert we have integer values correct, after update
  munit_assert_int(vec->data.v_vector3.x->data.v_int, ==, 2, "should have correct x");
  munit_assert_int(vec->data.v_vector3.y->data.v_int, ==, 2, "should have correct y");
  munit_assert_int(vec->data.v_vector3.z->data.v_int, ==, 2, "should have correct z");

  // Free all of our objects.
  free(i);
  free(vec);
  stats();
  assert(boot_all_freed());
});

int main() {
  MunitTest tests[] = {
      munit_test("/returns_null", test_returns_null),
      munit_test("/multiple_objects", test_vec_multiple_objects),
      munit_test("/same_object", test_vec_same_object),
      munit_null_test,
  };

  MunitSuite suite = munit_suite("object-vector", tests);

  return munit_suite_main(&suite, NULL, 0, NULL);
}
