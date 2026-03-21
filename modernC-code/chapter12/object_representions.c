// object_representions.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/17 14:22:25 PDT

//
// Created by Rob Ross on 3/17/26.
//

#include <limits.h>
#include <stdio.h>

typedef union unsignedInspect unsignedInspect; union unsignedInspect {
    unsigned val;
    unsigned char bytes[sizeof(unsigned)]; };

unsignedInspect twofold = { .val = 0xAABBCCDD, };


// num_bytes must be 1 - 8
// assumes little endian
unsigned long uint_for_bytes(unsigned num_bytes, const unsigned char bytes[static num_bytes], bool is_little_endian) {
    unsigned long result = 0;
    if (is_little_endian) {
        for (unsigned i = 0; i < num_bytes; ++i ) {
            result |= bytes[i] << (CHAR_BIT * i);
        }
    } else {
        for (unsigned i = 0; i < num_bytes; ++i ) {
            result |= bytes[i] << (CHAR_BIT * (num_bytes - 1 - i) );
        }
    }
    //((0xAA << (CHAR_BIT*3)) |(0xBB << (CHAR_BIT*2)) |(0xCC << CHAR_BIT) |0xDD)

    return result;
}

void temp(void) {
    // demonstrates little-endian-ness
    printf("value is 0x%.08X\n", twofold.val);
    for (size_t i = 0; i < sizeof twofold.bytes; ++i)
        printf("byte[%zu]: 0x%.02hhX\n", i, twofold.bytes[i]);

    printf("\n");

    unsigned int_value = uint_for_bytes( sizeof(unsigned), twofold.bytes, true);
    printf("value computed from little-endian bytes is 0x%.8X\n", int_value);

    unsigned char big_bytes[] = { 0xAA, 0xBB, 0xCC, 0xDD};

    int_value = uint_for_bytes( sizeof(unsigned), big_bytes, false);
    printf("value computed from big-endian bytes is 0x%.8X\n", int_value);


}


void func(void);    // declaration
void func(void) { } // definition

extern int x; // declaration
int x = 0;    // declaration AND definition

int y;  // declaration... definition too?

// at File Scope, int y; is a "tentative definition". In block scope it's a definition.

int y = 5;  // at file scope, this defines y.

int y;  // this is still a declaration so it's fine to include it multiple times
int y;

// int y = 7; // but can't do this since we already defined y = 5 above.

int main(void) {
    temp();
}