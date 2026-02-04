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

    printf("\nPrinting chars -128 to 127 as signed char\n");
    // are chars signed or unsigned?
    // On my Mac they are signed. so -128 to 127 in range. Which means they can't support extended ASCII.
    for (char c = -128; c < 127; c++) {
        printf("using %%i, %%c to print c : %%i: %i, %%c: %c\n", c, c);
    }
    // interesting. We can't add 1 to 127 (for signed char type) since that makes it -128. So in the loop above,
    // c < 128 is never true! We cannot check the "next higher" value in this edge case. So the loop must exit before
    // c == 127, and we must print the last value (127) here.
    printf("using %%i, %%c to print c : %%i: %i, %%c: %c\n", 127, 127);

    printf("\nPrinting chars 0 - 255 as unsigned char\n");
    for (unsigned char c = 0; c < 255; c++) {
        printf("using %%i, %%c to print c : %%i: %i, %%x: 0x%x, %%c: %c\n", c, c, c);

    }
    // interesting. We can't add 1 to 255 (for unsigned char type) since that makes it -128. So in the loop above,
    // c < 128 is never true! We cannot check the "next higher" value in this edge case. So the loop must exit before
    // c == 127, and we must print the last value (127) here.
    printf("using %%i, %%c to print c : %%i: %i, %%x: 0x%x, %%c: %c\n", 255, 255, 255);
}
