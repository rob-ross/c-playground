//
// Created by Rob Ross on 2/20/26.
//

// you cannot return an annonymous struct from a function. It must have a struct tag or typedef name
// does not compile:
//
// struct { int i1; } bar() {
//     return (struct { int i1; } ){.i1 = 42 };
// }

#include <stdio.h>

// but a prototype returning an annonymous struct is fine, apparently:
struct { int i1; } bar();


struct one_int { int i1; } foo() {

    return (struct one_int){ .i1 = 42};
}


void print_sum(int a, int b) {
    printf("%d + %d = %d\n", a, b, a + b);
}

void test_nullptr( [[maybe_unused]] char const str[static 1]) {
    // do things.
}

// make:
// clang -std=c23 -Wall -Wextra -Wconversion -Werror -o structs.out structs.c
int main() {
    [[maybe_unused]] struct one_int result = foo();

    // print_sum(1.5, 8.5);  // C allows implicit narrowing conversions for convertable types

    print_sum((int)1.5, (int)8.5);

     [[maybe_unused]] constexpr int n = 42;

    char const *s = NULL;
    test_nullptr(s);

}
