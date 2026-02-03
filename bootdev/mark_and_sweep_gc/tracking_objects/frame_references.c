


#include <assert.h>

#include "sneknew.h"
#include "vm.h"

#include "../../munit/munit.h"
#include "../../munit/munit_overrides.h"
#include "../../munit/memory_check.h"



// make:   clang -std=c17 frame_references.c sneknew.c snekobject.c stack.c vm.c ../../munit/munit.c ../../munit/memory_check.c -o frame_references.out

munit_case(RUN, test_one_ref, {
  vm_t *vm = vm_new();
  frame_t *frame = vm_new_frame(vm);

  snek_object_t *lanes_wpm = new_snek_integer(vm, 9);
  frame_reference_object(frame, lanes_wpm);

  munit_assert_int(frame->references->count, ==, 1, "Only one reference");
  munit_assert_ptr_equal(lanes_wpm, frame->references->data[0], "Refs lanes_wpm");

  vm_free(vm);
  assert(boot_all_freed());
});

munit_case(SUBMIT, test_multi_ref, {
  vm_t *vm = vm_new();
  frame_t *frame = vm_new_frame(vm);

  snek_object_t *lanes_wpm = new_snek_integer(vm, 9);
  snek_object_t *teej_wpm = new_snek_integer(vm, 160);
  frame_reference_object(frame, lanes_wpm);
  frame_reference_object(frame, teej_wpm);

  munit_assert_int(frame->references->count, ==, 2, "Two references");
  munit_assert_ptr_equal(lanes_wpm, frame->references->data[0], "Refs lanes_wpm");
  munit_assert_ptr_equal(teej_wpm, frame->references->data[1], "Refs teej_wpm");

  vm_free(vm);
  assert(boot_all_freed());
});

int main() {
    MunitTest tests[] = {
        munit_test("/test_one_ref", test_one_ref),
        munit_test("/test_multi_ref", test_multi_ref),
        munit_null_test,
    };

    MunitSuite suite = munit_suite("mark-and-sweep", tests);

    return munit_suite_main(&suite, NULL, 0, NULL);
}
