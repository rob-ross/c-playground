


#include <assert.h>

#include "vm.h"

#include "../munit/munit.h"
#include "../munit/munit_overrides.h"
#include "../munit/memory_check.h"

// make:   clang -std=c17 pros_and_cons.c stack.c vm.c snekobject.c ../munit/munit.c ../munit/memory_check.c -o pros_and_cons.out


munit_case(RUN, test_vm_new, {
  vm_t *vm = vm_new();
  munit_assert_int(vm->frames->capacity, ==, 8, "frames should have capacity 8");
  munit_assert_int(vm->objects->capacity, ==, 8, "objects should have capacity 8");
  vm_free(vm);
});

munit_case(SUBMIT, test_vm_new_free, {
  vm_t *vm = vm_new();
  vm_free(vm);
  assert(boot_all_freed());
});

int main() {
    MunitTest tests[] = {
        munit_test("/vm", test_vm_new),
        munit_test("/vm", test_vm_new_free),
        munit_null_test,
    };

    MunitSuite suite = munit_suite("mark-and-sweep", tests);

    return munit_suite_main(&suite, NULL, 0, NULL);
}
