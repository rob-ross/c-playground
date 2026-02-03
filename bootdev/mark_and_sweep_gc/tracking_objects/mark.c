


#include <assert.h>

#include "sneknew.h"
#include "vm.h"


#include "../../munit/munit.h"
#include "../../munit/munit_overrides.h"
#include "../../munit/memory_check.h"



// make:   clang -std=c17 mark.c sneknew.c snekobject.c stack.c vm.c ../../munit/munit.c ../../munit/memory_check.c -o mark.out

munit_case(RUN, test_single_frame, {
  vm_t *vm = vm_new();
  frame_t *frame = vm_new_frame(vm);

  snek_object_t *teej_skill = new_snek_integer(vm, 420);
  snek_object_t *lane_skill = new_snek_string(vm, "issues");

  mark(vm);
  // should not be marked because not in frame
  munit_assert_false(teej_skill->is_marked);
  munit_assert_false(lane_skill->is_marked);

  frame_reference_object(frame, teej_skill);
  frame_reference_object(frame, lane_skill);

  // after adding and marking, should be marked
  mark(vm);
  munit_assert_true(teej_skill->is_marked);
  munit_assert_true(lane_skill->is_marked);

  vm_free(vm);
  assert(boot_all_freed());
});

munit_case(SUBMIT, test_multi_frame, {
  vm_t *vm = vm_new();
  frame_t *frame = vm_new_frame(vm);
  frame_t *frame2 = vm_new_frame(vm);

  snek_object_t *teej_skill = new_snek_integer(vm, 420);
  snek_object_t *lane_skill = new_snek_string(vm, "issues");
  snek_object_t *prime_skill = new_snek_string(vm, "infinite");

  frame_reference_object(frame, teej_skill);
  frame_reference_object(frame, lane_skill);
  frame_reference_object(frame2, prime_skill);
  mark(vm);

  munit_assert_true(teej_skill->is_marked);
  munit_assert_true(lane_skill->is_marked);
  munit_assert_true(prime_skill->is_marked);
  vm_free(vm);
  assert(boot_all_freed());
});

int main() {
    MunitTest tests[] = {
        munit_test("/test_single_frame", test_single_frame),
        munit_test("/test_multi_frame", test_multi_frame),
        munit_null_test,
    };

    MunitSuite suite = munit_suite("mark-and-sweep", tests);

    return munit_suite_main(&suite, NULL, 0, NULL);
}
