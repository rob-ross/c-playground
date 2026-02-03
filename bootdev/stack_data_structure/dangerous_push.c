
#include "stdlib.h"
#include "snekstack.h"
#include "stack_includes.h"

// make:  clang dangerous_push.c snek_stack.c ../munit/munit.c -o dangerous_push.out

void scary_double_push(snekstack_t *s);

void scary_double_push(snekstack_t *s) {
    stack_push(s, (void*)1337);  // shhhh compiler. It's ok. Just sleep. Ignore the cast and sleep....
    int *int_ptr = malloc(sizeof(int));
    if (!int_ptr) {
        exit(1);
    }
    *int_ptr = 1024;
    stack_push(s, int_ptr);
}



munit_case(RUN, heterogenous_stack, {
  snekstack_t *s = stack_new(2);
  munit_assert_ptr_not_null(s, "Must allocate a new stack");

  scary_double_push(s);
  munit_assert_int(s->count, ==, 2, "Should have two items in the stack");

  int value = (int)s->data[0];
  munit_assert_int(value, ==, 1337, "Zero item should be 1337");

  int *pointer = s->data[1];
  munit_assert_int(*pointer, ==, 1024, "Top item should be 1024");

  free(pointer);
  stack_free(s);
});

int main() {
    MunitTest tests[] = {
        munit_test("/heterogenous_stack", heterogenous_stack),
        munit_null_test,
    };

    MunitSuite suite = munit_suite("snekstack", tests);

    return munit_suite_main(&suite, NULL, 0, NULL);
}

