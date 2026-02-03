

#include <stdio.h>

#include "snekobject.h"

#include "../munit/munit.h"
#include "../munit/munit_overrides.h"
#include "../munit/memory_check.h"


//make:  clang cycles.c snekobject.c ../munit/munit.c ../munit/memory_check.c -o cycles.out


munit_case(RUN, correctly_free, {
  snek_object_t *first = new_snek_array(1);
  snek_object_t *second = new_snek_array(1);
  // refcounts: first = 1, second = 1
  snek_array_set(first, 0, second);
  // refcounts: first = 1, second = 2
  snek_array_set(second, 0, first);
  // refcounts: first = 2, second = 2
  refcount_dec(first);
  refcount_dec(second);
  munit_assert_int(first->refcount, ==, 1, "Refcount first should be 1");
  munit_assert_int(second->refcount, ==, 1, "Refcount second should be 1");
});

// Don't touch below this line

int main() {
    MunitTest tests[] = {
        munit_test("/correctly_free", correctly_free),
        munit_null_test,
    };

    MunitSuite suite = munit_suite("refcount", tests);

    int result = munit_suite_main(&suite, NULL, 0, NULL);

    printf("*** NOTE: A memory leak warning is EXPECTED here ***\n");
    printf("*** We'll fix the circular reference problem soon ***\n");

    return result;
}
