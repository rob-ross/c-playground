//
// Created by Rob Ross on 2/6/26.
//

#include <stdlib.h>

#include "string_utils.h"
// make:   clang -std=c17 -o test_string_utils.out test_string_utils.c string_utils.c




void test_sutil_ends_with(void) {
    const char *str = "this is fun!";
    bool actual;

    actual = sutil_ends_with(str, "fun!");  // should return true
    printf("sutil_ends_with(%s, \"fun\") = %i, expected: 1\n", str, actual);

    actual = sutil_ends_with(str, "nope");  // should return false
    printf("sutil_ends_with(%s, \"nope\") = %i, expected: 0\n", str, actual);
}

void test_sutil_lower(void) {
    const char *original = "THIS IS A LOWERCASE STRING. WITH *&^( SOME %OTHER CHAR4CER$";
    const char *expected = "this is a lowercase string. with *&^( some %other char4cer$";
    char *actual = sutil_lower(original);
    printf("expected: %s, actual: %s, expected == actual: %i\n", expected, actual, sutil_strings_equal(expected, actual));
    free(actual);
}

void test_sutil_pad_center(void) {
    char *str1 = sutil_pad_center(" foo ", 10, '-');
    printf("'foo' centered 10 is : '%s'\n", str1);
    free(str1);
}

void test_sutil_pad_left(void) {
    const char *original = "foo";
    const int width = 10;
    const char fill_char = '*';
    const char *expected = "*******foo";

    char *actual = sutil_pad_left(original,width, fill_char);
    printf("expected: %s, actual: %s, expected == actual: %i\n", expected, actual, sutil_strings_equal(expected, actual));
    free(actual);
}

void test_sutil_pad_right(void) {
    const char *original = "foo";
    const int width = 10;
    const char fill_char = '*';
    const char *expected = "foo*******";

    char *actual = sutil_pad_right(original,width, fill_char);
    printf("expected: %s, actual: %s, expected == actual: %i\n", expected, actual, sutil_strings_equal(expected, actual));
    free(actual);
}

void test_sutil_starts_with(void) {
    const char *str = "this is fun!";
    bool actual;

    actual = sutil_starts_with(str, "this");  // should return true
    printf("sutil_starts_with(%s, \"this\") = %i, expected: 1\n", str, actual);

    actual = sutil_starts_with(str, "nope");  // should return false
    printf("sutil_starts_with(%s, \"nope\") = %i, expected: 0\n", str, actual);
}

void test_sutil_strip(void) {
    printf("\ntest_sutil_strip:\n");

    const char *original = "     five leading and trailing spaces     ";
    const char *expected = "five leading and trailing spaces";
    char *actual = sutil_strip(original, NULL);
    printf("expected: %s, actual: %s, expected == actual: %i\n", expected, actual, sutil_strings_equal(expected, actual));
    free(actual);

    actual = sutil_strip(original, "");
    printf("expected: %s, actual: %s, expected == actual: %i\n", expected, actual, sutil_strings_equal(expected, actual));
    free(actual);

    actual = sutil_strip(original, " ");
    printf("expected: %s, actual: %s, expected == actual: %i\n", expected, actual, sutil_strings_equal(expected, actual));
    free(actual);

    actual = sutil_strip(original, "*");
    printf("expected: %s, actual: %s, expected == actual: %i\n", original, actual, sutil_strings_equal(original, actual));
    free(actual);
}

void test_sutil_strip_left(void) {
    printf("\ntest_sutil_strip_left:\n");

    const char *original = "     five leading spaces";
    const char *expected = "five leading spaces";
    char *actual = sutil_strip_left(original, NULL);
    printf("expected: %s, actual: %s, expected == actual: %i\n", expected, actual, sutil_strings_equal(expected, actual));
    free(actual);

    actual = sutil_strip_left(original, "");
    printf("expected: %s, actual: %s, expected == actual: %i\n", expected, actual, sutil_strings_equal(expected, actual));
    free(actual);

    actual = sutil_strip_left(original, " ");
    printf("expected: %s, actual: %s, expected == actual: %i\n", expected, actual, sutil_strings_equal(expected, actual));
    free(actual);

    actual = sutil_strip_left(original, "*");
    printf("expected: %s, actual: %s, expected == actual: %i\n", original, actual, sutil_strings_equal(original, actual));
    free(actual);
}


void test_sutil_strip_right(void) {
    printf("\ntest_sutil_strip_right:\n");

    const char *original = "five trailing spaces     ";
    const char *expected = "five trailing spaces";
    char *actual = sutil_strip_right(original, NULL);
    printf("expected: %s, actual: %s, expected == actual: %i\n", expected, actual, sutil_strings_equal(expected, actual));
    free(actual);

    actual = sutil_strip_right(original, "");
    printf("expected: %s, actual: %s, expected == actual: %i\n", expected, actual, sutil_strings_equal(expected, actual));
    free(actual);

    actual = sutil_strip_right(original, " ");
    printf("expected: %s, actual: %s, expected == actual: %i\n", expected, actual, sutil_strings_equal(expected, actual));
    free(actual);

    actual = sutil_strip_right(original, "*");
    printf("expected: %s, actual: %s, expected == actual: %i\n", original, actual, sutil_strings_equal(original, actual));
    free(actual);
}

void test_sutil_upper(void) {
    const char *original = "this is a lowercase string. with *&^( some %other char4cer$";
    const char *expected = "THIS IS A LOWERCASE STRING. WITH *&^( SOME %OTHER CHAR4CER$";
    char *actual = sutil_upper(original);
    printf("expected: %s, actual: %s, expected == actual: %i\n", expected, actual, sutil_strings_equal(expected, actual));
    free(actual);
}

// make:   clang -std=c17 -o test_string_utils.out test_string_utils.c string_utils.c

int main(int argc, char *argv[]) {
    test_sutil_ends_with();
    test_sutil_lower();
    test_sutil_pad_center();
    test_sutil_pad_left();
    test_sutil_pad_right();
    test_sutil_starts_with();
    test_sutil_strip();
    test_sutil_strip_left();
    test_sutil_strip_right();
    test_sutil_upper();
}
