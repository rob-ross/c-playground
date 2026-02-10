//
// Created by Rob Ross on 2/9/26.
//
// Exercise 1.8, page 20
// Write a program to count blanks, tabs, and newlines.
// Enter control-D to enter end-of-file

#include <stdio.h>

// make:
// clang -std=c17 -o ex_1.8.out ex_1.8.c
int main(void) {
    int num_blanks = 0;
    int num_tabs = 0;
    int num_newlines = 0;

    printf("Enter some text and I will count blanks, tabs, and newlines for you : ");
    int c;
    while ( ( c = getchar()) != EOF) {
        if ( c == ' ') num_blanks++;
        if (c == '\t') num_tabs++;
        if (c == '\n') num_newlines++;
    }
    printf("\nnum_blanks: %i\nnum_tabs:%i\nnum_newlines: %i\n", num_blanks, num_tabs, num_newlines);
}