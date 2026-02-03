

#include <assert.h>

#include "snekobject.h"
#include <stdio.h>
#include <stdlib.h>

#include "../munit/munit.h"
#include "../munit/munit_overrides.h"
#include "../munit/memory_check.h"


//make:  clang vectors.c snekobject.c ../munit/munit.c ../munit/memory_check.c -o vectors.out


munit_case(RUN, test_vector3_refcounting, {
  snek_object_t *foo = new_snek_integer(1);
  snek_object_t *bar = new_snek_integer(2);
  snek_object_t *baz = new_snek_integer(3);

  snek_object_t *vec = new_snek_vector3(foo, bar, baz);
  munit_assert_int(foo->refcount, ==, 2, "foo is now referenced by vec");
  munit_assert_int(bar->refcount, ==, 2, "bar is now referenced by vec");
  munit_assert_int(baz->refcount, ==, 2, "baz is now referenced by vec");

  // `foo` is stil referenced in the `vec`, so it should not be freed.
  refcount_dec(foo);
  // assert(!boot_is_freed(foo));

  refcount_dec(vec);
  // assert(boot_is_freed(foo));

  // These are still alive, they have the original reference still.
  // assert(!boot_is_freed(bar));
  // assert(!boot_is_freed(baz));

  // Decrement the last reference to the objects, so they will be freed.
  refcount_dec(bar);
  refcount_dec(baz);

  assert(boot_all_freed());
});

munit_case(SUBMIT, test_vector3_refcounting_same, {
  snek_object_t *foo = new_snek_integer(1);

  snek_object_t *vec = new_snek_vector3(foo, foo, foo);
  munit_assert_int(foo->refcount, ==, 4, "foo is now referenced by vec x3");

  // `foo` is stil referenced in the `vec`, so it should not be freed.
  refcount_dec(foo);
  // assert(!boot_is_freed(foo));

  refcount_dec(vec);
  // assert(boot_is_freed(foo));
  // assert(boot_is_freed(vec));
  assert(boot_all_freed());
});

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

munit_case(RUN, test_refcount_free_is_called, {
  snek_object_t *obj = new_snek_float(4.20);

  refcount_inc(obj);
  munit_assert_int(obj->refcount, ==, 2, "Refcount should be incremented");

  refcount_dec(obj);
  munit_assert_int(obj->refcount, ==, 1, "Refcount should be decremented");

  refcount_dec(obj);
  // assert(boot_is_freed(obj));
  assert(boot_all_freed());
});

munit_case(RUN, test_allocated_string_is_freed, {
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
      munit_test("/vector3", test_vector3_refcounting),
      munit_test("/vector3-same", test_vector3_refcounting_same),
      munit_null_test,
  };

  MunitSuite suite = munit_suite("refcount", tests);

  return munit_suite_main(&suite, NULL, 0, NULL);
}
