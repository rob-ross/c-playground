//
// Created by Rob Ross on 2/11/26.
//

#include <stdio.h>

// extern int some_function(char *c);

// make:
// clang -std=c17 -o file.out file1.c file2.c

int main(void) {
    int i = 0;

    // no prototype for some_function in a header file. Without this declaration, the compiler complains about the
    // method call
    extern int some_function(char *c);  // `extern` keyword is optional here.
    /// I.e., `int some_function(char *c);` works as well.

    // not visible without the above declaration.
    i = some_function("foo");

    printf("i = %d\n", i);
}