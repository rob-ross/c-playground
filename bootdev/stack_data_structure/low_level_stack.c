


#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "stack_includes.h"


// make clang low_level_stack.c ../munit/munit.c -o low_level_stack.out

typedef struct Stack {
    size_t count;
    size_t capacity;
    void **data;
} my_stack_t;

my_stack_t *stack_new(size_t capacity) {
    my_stack_t *stack = malloc(sizeof(my_stack_t)) ;
    if (!stack) {
        return NULL;
    }
    void *data = malloc(capacity * sizeof(size_t));
    if (!data) {
        free(stack);
        return NULL;
    }
    stack->count = 0;
    stack->capacity = capacity;
    stack->data = data;
    return stack;
}


my_stack_t *stack_new(size_t capacity);

munit_case(RUN, create_stack_small, {
  my_stack_t *s = stack_new(3);
  munit_assert_int(s->capacity, ==, 3, "Sets capacity to 3");
  munit_assert_int(s->count, ==, 0, "No elements in the stack yet");
  munit_assert_ptr_not_null(s->data, "Allocates the stack data");

  free(s->data);
  free(s);

  assert(boot_all_freed());
});

munit_case(SUBMIT, create_stack_large, {
  my_stack_t *s = stack_new(100);
  munit_assert_int(s->capacity, ==, 100, "Sets capacity to 100");
  munit_assert_int(s->count, ==, 0, "No elements in the stack yet");
  munit_assert_ptr_not_null(s->data, "Allocates the stack data");

  free(s->data);
  free(s);

  assert(boot_all_freed());
});

int main() {
    MunitTest tests[] = {
        munit_test("/create_stack_small", create_stack_small),
        munit_test("/create_stack_large", create_stack_large),
        munit_null_test,
    };

    MunitSuite suite = munit_suite("snekstack", tests);

    return munit_suite_main(&suite, NULL, 0, NULL);
}
