//
// Created by Rob Ross on 2/9/26.
//
// Exercise 1-9 page 20
// Write a program to copy its input to its output, replacing each string of one or more blanks by a single blank.
#include <stdio.h>



// make:
// clang -std=c17 -o ex_1.9.out ex_1.9.c

int main(void) {

    int blank_found = 0; // 1 == true
    printf("Enter some text and I will convert multiple blanks to a single blank: ");
    int c = 0;
    while ( ( c = getchar() ) != EOF) {
        if (c == ' ') {
            putchar(c);
            // eat additinal blanks
            while ( (c = getchar() ) != EOF && c == ' ');
            putchar(c);  // this isn't a blank line.
        } else {
            putchar(c);
        }
    }
}