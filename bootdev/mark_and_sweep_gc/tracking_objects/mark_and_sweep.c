


#include <assert.h>

#include "sneknew.h"
#include "vm.h"

#include "../../munit/munit.h"
#include "../../munit/munit_overrides.h"
#include "../../munit/memory_check.h"



// make:   clang -std=c17 mark_and_sweep.c sneknew.c snekobject.c stack.c vm.c ../../munit/munit.c ../../munit/memory_check.c -o mark_and_sweep.out

munit_case(RUN, test_field_exists, {
  vm_t *vm = vm_new();
  snek_object_t *lane_courses = new_snek_integer(vm, 20);
  snek_object_t *teej_courses = new_snek_integer(vm, 1);
  (void)lane_courses->is_marked;
  (void)teej_courses->is_marked;
  vm_free(vm);
  assert(boot_all_freed());
});

munit_case(SUBMIT, test_marked_is_false, {
  vm_t *vm = vm_new();
  snek_object_t *lane_courses = new_snek_integer(vm, 20);
  snek_object_t *teej_courses = new_snek_integer(vm, 1);
  munit_assert_false(lane_courses->is_marked);
  munit_assert_false(teej_courses->is_marked);
  vm_free(vm);
  assert(boot_all_freed());
});

int main() {
    MunitTest tests[] = {
        munit_test("/test_field_exists", test_field_exists),
        munit_test("/test_marked_is_false", test_marked_is_false),
        munit_null_test,
    };

    MunitSuite suite = munit_suite("mark-and-sweep", tests);

    return munit_suite_main(&suite, NULL, 0, NULL);
}
