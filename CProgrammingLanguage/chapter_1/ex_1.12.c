//
// Created by Rob Ross on 2/9/26.
//
// Excersise 1-12 page 21
// Write a program that prints its input one word per line.

// make:
// clang -std=c17 -o ex_1.12.out ex_1.12.c

#include <stdio.h>

#define IN 1    // inside a word
#define OUT 0   // outside a word


int foo(int);

int main(void) {

    int c = 0;
    printf("Enter words and I'll display them one per line.\n");
    printf("input: ");
    while ( (c = getchar()) != EOF ) {
        if ( c == ' ' || c == '\n' || c == '\t' ) {
            putchar('\n');
        } else  {
            putchar(c);
        }


        if (c == '\n') {
            printf("input: ");
        }
    }


    return 0;
}

