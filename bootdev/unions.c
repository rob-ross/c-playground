//
// Created by Rob Ross on 1/29/26.
//

#include <stdio.h>
#include "munit/munit.h"
#include "munit/munit_overrides.h"

//make : clang unions.c munit/munit.c -o unions.out

typedef enum SnekObjectKind {
    INTEGER,
    STRING
} snek_object_kind_t;

// don't touch below this line'

typedef union SnekObjectData {
    int v_int;
    char *v_string;
} snek_object_data_t;

typedef struct SnekObject {
    snek_object_kind_t kind;
    snek_object_data_t data;
} snek_object_t;

snek_object_t new_integer(int);
snek_object_t new_string(char *str);
void format_object(snek_object_t obj, char *buffer);

void format_object(snek_object_t obj, char *buffer) {
    switch (obj.kind) {
        case INTEGER:
            sprintf(buffer, "int:%d", obj.data.v_int  );
            break;
        case STRING:
            sprintf(buffer, "string:%s", obj.data.v_string);
            break;
    }
}

// don't touch below this line'

snek_object_t new_integer(int i) {
    return (snek_object_t){
        .kind = INTEGER,
        .data = {.v_int = i},
    };
}

snek_object_t new_string(char *str) {
    // NOTE: We will learn how to copy this data later.
    return (snek_object_t){
        .kind = STRING,
        .data = {.v_string = str},
    };
}

munit_case(RUN, test_formats_int1, {
  char buffer[100];
  snek_object_t i = new_integer(5);
  format_object(i, buffer);

  munit_assert_string_equal("int:5", buffer, "formats INTEGER");
});

munit_case(RUN, test_formats_string1, {
  char buffer[100];
  snek_object_t s = new_string("Hello!");
  format_object(s, buffer);

  munit_assert_string_equal("string:Hello!", buffer, "formats STRING");
});

munit_case(SUBMIT, test_formats_int2, {
  char buffer[100];
  snek_object_t i = new_integer(2014);
  format_object(i, buffer);

  munit_assert_string_equal("int:2014", buffer, "formats INTEGER");
});

munit_case(SUBMIT, test_formats_string2, {
  char buffer[100];
  snek_object_t s = new_string("nvim btw");
  format_object(s, buffer);

  munit_assert_string_equal("string:nvim btw", buffer, "formats STRING");
});

int main() {
    MunitTest tests[] = {
        munit_test("/integer", test_formats_int1),
        munit_test("/string", test_formats_string1),
        munit_test("/integer_nvim", test_formats_int2),
        munit_test("/string_nvim", test_formats_string2),
        munit_null_test,
    };

    MunitSuite suite = munit_suite("format", tests);

    return munit_suite_main(&suite, NULL, 0, NULL);
}