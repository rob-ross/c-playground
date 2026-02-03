

#include <string.h>
#include <stdlib.h>

#include "snekstack.h"
#include "stack_includes.h"

// make:  clang multiple_types.c snek_stack.c ../munit/munit.c -o multiple_types.out


void stack_push_multiple_types(snekstack_t *s);


void stack_push_multiple_types(snekstack_t *s) {
    float *float_ptr = malloc(sizeof(float));
    if (!float_ptr) {
        exit(1);
    }
    *float_ptr = 3.14f;
    stack_push(s, float_ptr);

    const char *msg = "Sneklang is blazingly slow!";
    size_t msg_len = strlen(msg);
    char *char_ptr = malloc(msg_len);
    if (!char_ptr) {
        exit(1);
    }
    strncpy(char_ptr, msg, msg_len);
    stack_push(s, char_ptr);

}



munit_case(RUN, multiple_types_stack, {
  snekstack_t *s = stack_new(4);
  munit_assert_ptr_not_null(s, "Must allocate a new stack");

  stack_push_multiple_types(s);
  munit_assert_int(s->count, ==, 2, "Should have two items in the stack");

  float *f = s->data[0];
  munit_assert_float(*f, ==, 3.14, "Float is equal");

  char *string = s->data[1];
  munit_assert_string_equal(string, "Sneklang is blazingly slow!", "char* is equal");

  free(f);
  free(string);
  stack_free(s);
});

int main() {
    MunitTest tests[] = {
        munit_test("/multiple_types_stack", multiple_types_stack),
        munit_null_test,
    };

    MunitSuite suite = munit_suite("snekstack", tests);

    return munit_suite_main(&suite, NULL, 0, NULL);
}
