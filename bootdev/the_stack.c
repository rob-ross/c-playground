//
// Created by Rob Ross on 1/29/26.
//
// clang the_stack.c munit/munit.c -o the_stack.out

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "./munit/munit.h"
#include "munit/munit_overrides.h"

void printMessageOne();
void printMessageTwo();
void printMessageThree();
void printStackPointerDiff();


int main() {
    printMessageOne();
    return 0;
}

void printMessageOne() {
    const char *message = "Dark mode?\n";
    printStackPointerDiff();
    printf("%s\n", message);
    printMessageTwo();
}

void printMessageTwo() {
    const char *message = "More like...\n";
    printStackPointerDiff();
    printf("%s\n", message);
    printMessageThree();
}

void printMessageThree() {
    const char *message = "dark roast.\n";
    printStackPointerDiff();
    printf("%s\n", message);
}

// don't touch below this line

void printStackPointerDiff() {
    static void *last_sp = NULL;
    void *current_sp;
    current_sp = __builtin_frame_address(0);
    long diff = (char *)last_sp - (char *)current_sp;
    if (last_sp == NULL) {
        last_sp = current_sp;
        diff = 0;
    }
    printf("---------------------------------\n");
    printf("Stack pointer offset: %ld bytes\n", diff);
    printf("---------------------------------\n");
}