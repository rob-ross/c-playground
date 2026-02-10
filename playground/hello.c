
//
// Created by Rob Ross on 2/8/26.
//


#include <stdio.h>

int main() {
    printf("Hello, world!\n");

    printf("EOF=%i\n", EOF);
    int c;

    while ( (c = getchar()) != EOF ) {
        putchar(c);
    }
    return 0;
}
