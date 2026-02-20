//
// Created by Rob Ross on 2/11/26.
//


#include <stdarg.h>
#include <stdio.h>
#include <limits.h>

// Define a sentinel value that is unlikely to be a valid input.
#define END_OF_ARGS INT_MIN

// Use a macro to automatically append the sentinel to the argument list.
#define display_ints(...) display_ints_impl(__VA_ARGS__, END_OF_ARGS)

// prints the ints in the argument list. INT_MIN is used as a sentinel value to signal end of the argument list.
// thus INT_MIN is not supported as a discrete argument value.
void display_ints_impl(const int i1, ...) {
    va_list argv;

    printf("display_ints: { %d", i1);
    va_start(argv, i1);

    int i = 0;
    while ( ( i = va_arg(argv, int)) != END_OF_ARGS ) {
        printf(", %d", i);
    }

    printf(" }\n");  // closing brace, end of list
    va_end(argv); // must have a va_end for every va_start
}

int main(int argc, char *argv[]) {
    display_ints(1,2,3);
    display_ints(4,3,2,1);
}
