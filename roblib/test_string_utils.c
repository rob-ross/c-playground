//
// Created by Rob Ross on 2/6/26.
//

#include "string_utils.h"
// make:   clang -std=c17 -o test_string_utils.out test_string_utils.c string_utils.c


void test_sutil_pad_left(void) {
    const char *original = "foo";
    const int width = 10;
    const char fill_char = '*';
    const char *expected = "*******foo";

    char *actual = sutil_pad_left(original,width, fill_char);
    printf("expected: %s, actual: %s, expected == actual: %i\n", expected, actual, sutil_strings_equal(expected, actual));
}

int main(int argc, char *argv[]) {
    test_sutil_pad_left();
}
