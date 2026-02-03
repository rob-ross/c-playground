
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "sneknew.h"
#include "vm.h"

#include "../../munit/munit.h"
#include "../../munit/munit_overrides.h"
#include "../../munit/memory_check.h"



// make:   clang -std=c17 free.c sneknew.c snekobject.c stack.c vm.c ../../munit/munit.c ../../munit/memory_check.c -o free.out

munit_case(RUN, test_reference_object, {
    vm_t *vm = vm_new();
    new_snek_integer(vm, 5);
    new_snek_string(vm, "hello");
    vm_free(vm);

    assert(boot_all_freed());
});

munit_case(SUBMIT, test_frames_are_freed, {
  vm_t *vm = vm_new();
  vm_new_frame(vm);
  vm_new_frame(vm);
  vm_free(vm);
  assert(boot_all_freed());
});

int main() {
    MunitTest tests[] = {
        munit_test("/test_reference_object", test_reference_object),
        munit_test("/test_frames_are_freed", test_frames_are_freed),
        munit_null_test,
    };

    MunitSuite suite = munit_suite("mark-and-sweep", tests);

    return munit_suite_main(&suite, NULL, 0, NULL);
}
