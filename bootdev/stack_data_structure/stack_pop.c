


#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "stack_includes.h"



// make: clang stack_pop.c ../munit/munit.c -o stack_pop.out

typedef struct Stack {
  size_t count;
  size_t capacity;
  void **data;
} _mystack_t;

_mystack_t *stack_new(size_t capacity);
void stack_push(_mystack_t *stack, void *obj);
void *stack_pop(_mystack_t *stack);


void *stack_pop(_mystack_t *stack) {
  if (stack->count == 0) {
    return NULL;
  }
  stack->count--;
  return stack->data[stack->count];
}

// don't touch below this line'

void stack_push(_mystack_t *stack, void *obj) {
  if (stack->count == stack->capacity) {
    stack->capacity *= 2;
    void **temp = realloc(stack->data, stack->capacity * sizeof(void *));
    if (temp == NULL) {
      stack->capacity /= 2;

      exit(1);
    }
    stack->data = temp;
  }
  stack->data[stack->count] = obj;
  stack->count++;
}

_mystack_t *stack_new(size_t capacity) {
  _mystack_t *stack = malloc(sizeof(_mystack_t));
  if (stack == NULL) {
    return NULL;
  }

  stack->count = 0;
  stack->capacity = capacity;
  stack->data = malloc(stack->capacity * sizeof(void *));
  if (stack->data == NULL) {
    free(stack);
    return NULL;
  }

  return stack;
}


munit_case(RUN, pop_stack, {
  _mystack_t *s = stack_new(2);
  munit_assert_ptr_not_null(s, "Must allocate a new stack");

  munit_assert_int(s->capacity, ==, 2, "Sets capacity to 2");
  munit_assert_int(s->count, ==, 0, "No elements in the stack yet");
  munit_assert_ptr_not_null(s->data, "Allocates the stack data");

  int one = 1;
  int two = 2;
  int three = 3;

  stack_push(s, &one);
  stack_push(s, &two);

  munit_assert_int(s->capacity, ==, 2, "Sets capacity to 2");
  munit_assert_int(s->count, ==, 2, "2 elements in the stack");

  stack_push(s, &three);
  munit_assert_int(s->capacity, ==, 4, "Capacity is doubled");
  munit_assert_int(s->count, ==, 3, "3 elements in the stack");

  int *popped = stack_pop(s);
  munit_assert_int(*popped, ==, three, "Should pop the last element");

  popped = stack_pop(s);
  munit_assert_int(*popped, ==, two, "Should pop the last element");

  popped = stack_pop(s);
  munit_assert_int(*popped, ==, one, "Should pop the only remaining element");

  popped = stack_pop(s);
  munit_assert_null(popped, "No remaining elements");

  // Clean up our allocated data.
  free(s->data);
  free(s);

  // Should be nothing left that is allocated.
  assert(boot_all_freed());
});

munit_case(SUBMIT, pop_stack_empty, {
  _mystack_t *s = stack_new(2);
  munit_assert_ptr_not_null(s, "Must allocate a new stack");

  munit_assert_int(s->capacity, ==, 2, "Sets capacity to 2");
  munit_assert_int(s->count, ==, 0, "No elements in the stack yet");
  munit_assert_ptr_not_null(s->data, "Allocates the stack data");

  int *popped = stack_pop(s);
  munit_assert_null(popped, "Should return null when popping an empty stack");

  // Clean up our allocated data.
  free(s->data);
  free(s);

  // Should be nothing left that is allocated.
  assert(boot_all_freed());
});

munit_case(RUN, push_stack, {
  _mystack_t *s = stack_new(2);
  munit_assert_ptr_not_null(s, "Must allocate a new stack");

  munit_assert_int(s->capacity, ==, 2, "Sets capacity to 2");
  munit_assert_int(s->count, ==, 0, "No elements in the stack yet");
  munit_assert_ptr_not_null(s->data, "Allocates the stack data");

  int a = 1;

  stack_push(s, &a);
  stack_push(s, &a);

  munit_assert_int(s->capacity, ==, 2, "Sets capacity to 2");
  munit_assert_int(s->count, ==, 2, "2 elements in the stack");

  stack_push(s, &a);
  munit_assert_int(s->capacity, ==, 4, "Capacity is doubled");
  munit_assert_int(s->count, ==, 3, "3 elements in the stack");

  // Clean up our allocated data.
  free(s->data);
  free(s);

  // Should be nothing left that is allocated.
  assert(boot_all_freed());
});

munit_case(RUN, create_stack, {
  _mystack_t *s = stack_new(10);
  munit_assert_int(s->capacity, ==, 10, "Sets capacity to 10");
  munit_assert_int(s->count, ==, 0, "No elements in the stack yet");
  munit_assert_ptr_not_null(s->data, "Allocates the stack data");

  // Clean up our allocated data.
  free(s->data);
  free(s);

  // Should be nothing left that is allocated.
  assert(boot_all_freed());
});

int main() {
  MunitTest tests[] = {
      munit_test("/create_stack", create_stack),
      munit_test("/push_stack", push_stack),
      munit_test("/pop_stack", pop_stack),
      munit_test("/pop_stack_empty", pop_stack_empty),
      munit_null_test,
  };

  MunitSuite suite = munit_suite("snekstack", tests);

  return munit_suite_main(&suite, NULL, 0, NULL);
}
