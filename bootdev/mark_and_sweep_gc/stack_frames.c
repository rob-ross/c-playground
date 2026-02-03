#include <assert.h>
#include <stdlib.h>

#include "vm.h"

#include "../munit/munit.h"
#include "../munit/munit_overrides.h"
#include "../munit/memory_check.h"

// make:   clang -std=c17 stack_frames.c stack.c vm.c snekobject.c ../munit/munit.c ../munit/memory_check.c -o stack_frames.out


munit_case(RUN, test_vm_new, {
  vm_t *vm = vm_new();
  vm_new_frame(vm);
  munit_assert_int(vm->frames->count, ==, 1, "frame was pushed");
  vm_free(vm);
});

munit_case(RUN, test_vm_new_frame, {
  vm_t *vm = vm_new();
  frame_t *frame = vm_new_frame(vm);
  munit_assert_ptr_not_equal(frame->references, NULL,
             "frame->references must be allocated");
  munit_assert_int(frame->references->count, ==, 0,
             "references stack should start empty");
  assert(frame->references->capacity >
         0); // references stack must have capacity > 0
  munit_assert_ptr_not_equal(frame->references->data, NULL,
             "references stack backing array must be allocated");
  vm_free(vm);
});

munit_case(RUN, test_frames_are_freed, {
    clear_stats();
    vm_t *vm = vm_new();
    vm_new_frame(vm);
    vm_free(vm);
    assert(boot_all_freed());
});

int main() {
    MunitTest tests[] = {
        munit_test("/test_vm_new", test_vm_new),
        munit_test("/test_vm_new_frame", test_vm_new_frame),
        munit_test("/test_frames_are_freed", test_frames_are_freed),
        munit_null_test,
    };

    MunitSuite suite = munit_suite("mark-and-sweep", tests);

    return munit_suite_main(&suite, NULL, 0, NULL);
}
