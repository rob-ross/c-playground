

#include <assert.h>

#include "sneknew.h"
#include "snekobject.h"
#include "vm.h"

#include "../../munit/munit.h"
#include "../../munit/munit_overrides.h"
#include "../../munit/memory_check.h"

// make:    clang -std=c17 tracking_objects.c sneknew.c stack.c vm.c ../../munit/munit.c ../../munit/memory_check.c -o tracking_objects.out

munit_case(RUN, test_new_object, {
    clear_stats();
    vm_t *vm = vm_new();
    snek_object_t *obj = new_snek_integer(vm, 5);
    munit_assert_int(obj->kind, ==, INTEGER, "kind must be INTEGER");
    munit_assert_ptr_equal(vm->objects->data[0], obj, "object must be tracked");
    free(obj);
    vm_free(vm);
    assert(boot_all_freed());
});

munit_case(RUN, test_vm_new, {
  vm_t *vm = vm_new();
  munit_assert_ptr_not_null(vm->frames, "frames must not be NULL");
  munit_assert_ptr_not_null(vm->objects, "objects must not be NULL");
  vm_free(vm);
  assert(boot_all_freed());
});

munit_case(RUN, test_frames_are_freed, {
    vm_t *vm = vm_new();
    vm_new_frame(vm);
    vm_free(vm);
    assert(boot_all_freed());
});

int main() {
    MunitTest tests[] = {
        munit_test("/test_vm_new", test_vm_new),
        munit_test("/test_frames_are_freed", test_frames_are_freed),
        munit_test("/test_new_object", test_new_object),
        munit_null_test,
    };

    MunitSuite suite = munit_suite("mark-and-sweep", tests);

    return munit_suite_main(&suite, NULL, 0, NULL);
}
