//
// Created by Rob Ross on 2/9/26.
//
#include <stdio.h>

// Exercise 1-10 page 20
// Write a program to copy its input to its output, replacing each tab by \t, each backspace by \b, and each
// backslash by \\. Note, control-H is backspace, but the terminal will eat it. So you must enter control-V first.
// control-V means "treat next key literally".

// make:
// clang -std=c17 -o ex_1.10.out ex_1.10.c

int main(void) {

    printf("Enter some text and will substitute for tabs, backspaces, and backslashes: \n");
    int c;
    while ((c = getchar()) != EOF) {
        if (c == '\t') {
            putchar('\\');
            putchar('t');
        } else if (c == '\b' ) {
            putchar('\\');
            putchar('b');
        } else if (c == '\\' ) {
            putchar('\\');
            putchar('\\');
        } else {
            putchar(c);
        }
        if (c == '\n') {
            printf("Input: ");
        }
    }
    return 0;
}
