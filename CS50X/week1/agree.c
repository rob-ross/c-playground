//
// Created by Rob Ross on 1/4/26.
//
#include "../cs50.h"
#include <stdio.h>

int main(void) {
    char c = get_char("Do you wanna Fanta? (y/n): ");

    if (c == 'y' || c == 'Y') {
        printf("YESSSS!!!!!\n");
    } else {
        printf("Well your loss, Sonny\n");
    }
}