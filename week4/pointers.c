//
// Created by Rob Ross on 1/24/26.
//

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint8_t ubyte;

int main(void) {
    char *c = "Hello!";

    printf("using %%s to print char*: %s\n", c);
    printf("using %%c to print char*: %c\n", c);
    // c is a memory address, %c tries to format it as a char, which is an unsigned byte.
    printf("using %%p to print char*: %p\n", c);
    // %c above is using the last byte of this address as the ordinal for a character
    ubyte last_byte = (uintptr_t)c & 0x000000FF;
    printf("using %%c to print last byte of %p as ubyte: %c\n", c, last_byte);
    // Output:
    // using %s to print char*: Hello!
    // using %c to print char*: &
    // using %p to print char*: 0x10b99cf26
    // using %c to print last byte of 0x10b99cf26 as ubyte: &

    printf("\n");

    for ( int i = 0; i < strlen(c); i++) {
        printf("using %%c to print c[%i]: %c\n", i, c[i]);
    }
    printf("\n");
    for ( int i = 0; i < strlen(c); i++) {
        printf("using %%c to print *(c + %i): %c\n", i, *(c + i));
    }

    printf("\n");
    for ( int i = 0; i < strlen(c); i++) {
        printf("using %%s to print c + %i: %s\n", i, c + i);
    }
}
