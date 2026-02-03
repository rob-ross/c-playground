


#include <assert.h>

#include "snekobject.h"
#include <stdio.h>
#include <stdlib.h>

#include "../munit/munit.h"
#include "../munit/munit_overrides.h"
#include "../munit/memory_check.h"

//make:  clang decrement_and_free.c snekobject.c ../munit/munit.c ../munit/memory_check.c -o decrement_and_free.out

munit_case(RUN, test_int_has_refcount, {
  snek_object_t *obj = new_snek_integer(10);
  munit_assert_int(obj->refcount, ==, 1, "Refcount should be 1 on creation");

  free(obj);
});

munit_case(RUN, test_inc_refcount, {
  snek_object_t *obj = new_snek_float(4.20);
  munit_assert_int(obj->refcount, ==, 1, "Refcount should be 1 on creation");

  refcount_inc(obj);
  munit_assert_int(obj->refcount, ==, 2, "Refcount should be incremented");

  free(obj);
});

munit_case(RUN, test_dec_refcount, {
  snek_object_t *obj = new_snek_float(4.20);

  refcount_inc(obj);
  munit_assert_int(obj->refcount, ==, 2, "Refcount should be incremented");

  refcount_dec(obj);
  munit_assert_int(obj->refcount, ==, 1, "Refcount should be decremented");

  // assert(!boot_is_freed(obj));

  // Object is still alive, so we will free manually.
  free(obj);
});

munit_case(SUBMIT, test_refcount_free_is_called, {
  snek_object_t *obj = new_snek_float(4.20);

  refcount_inc(obj);
  munit_assert_int(obj->refcount, ==, 2, "Refcount should be incremented");

  refcount_dec(obj);
  munit_assert_int(obj->refcount, ==, 1, "Refcount should be decremented");

  refcount_dec(obj);
  // assert(boot_is_freed(obj));
  assert(boot_all_freed());
});

munit_case(SUBMIT, test_allocated_string_is_freed, {
  snek_object_t *obj = new_snek_string("Hello @wagslane!");

  refcount_inc(obj);
  munit_assert_int(obj->refcount, ==, 2, "Refcount should be incremented");

  refcount_dec(obj);
  munit_assert_int(obj->refcount, ==, 1, "Refcount should be decremented");
  munit_assert_string_equal(obj->data.v_string, "Hello @wagslane!", "references str");

  refcount_dec(obj);
  // assert(boot_is_freed(obj));
  assert(boot_all_freed());
});

int main() {
  MunitTest tests[] = {
      munit_test("/has_refcount", test_int_has_refcount),
      munit_test("/inc_refcount", test_inc_refcount),
      munit_test("/dec_refcount", test_dec_refcount),
      munit_test("/free_refcount", test_refcount_free_is_called),
      munit_test("/string_freed", test_allocated_string_is_freed),
      munit_null_test,
  };

  MunitSuite suite = munit_suite("refcount", tests);

  return munit_suite_main(&suite, NULL, 0, NULL);
}
