// const_tests.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/06/04 13:24:45 PDT

#include <stdio.h>

#define B 10

const int A = 10;
const int C = A + B;

const int D = A + 1;

const int E = A + C;   // clever compiler! still a const


int main(void) {

    // B is a macro
    printf("B = %d\n", B);  // output: B = 10

    printf("&B = %p\n", B); // output: &B = 0xa  ; B interpreted as int, formatted as pointer

    //printf("&B = %p\n", &B);  // output: error: cannot take the address of an rvalue of type 'int'


    // try to modify const var A:
    printf("A = %d, &A=%p\n", A, &A);  // A = 10, &A=0x101961f70

    //A = 5; // trivially a compiler erorr: error: cannot assign to variable 'A' with const-qualified type 'const int'

    // on my system const int A = 10; goes into protected memory
    //*((int *)&A) = 5;  // Process finished with exit code 138 (interrupted by signal 10:SIGBUS)
    // printf("A = %d, &A=%p\n", A, &A);



}
