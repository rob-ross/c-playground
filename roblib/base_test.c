//
// Created by Rob Ross on 2/21/26.
//


#include <stdio.h>
#include "base.h"


//make:
// clang -std=c23 -o base_test.out base_test.c
int main(void) {

    bool b = false;
    int foo = 42;
    double bar = 42.42;
    char c = '4';

    bool flag = false;

    PVL(b);
    PVL(true);
    PVL(foo);

    PVL(32 - 124.68);
    PVL(bar);
    PVL(c);

    char *str = "hello";
    PVL(str);

    char const *str2 = "world";
    PVL(str2);

    int const iconst = 99;

    PVL(iconst);

    PVL(&iconst);

    PVL(&foo);

    short shortie = 9;
    PVL(shortie);
    PVL(&shortie);
    PVL((short const*)&shortie);

    void *voider;
    void const* constvoid;

    PVL(voider);
    PVL(constvoid);

    size_t sz = 24;
    PVL(sz);
    printf("sz = %zu\n", sz);

    bool *bool_p = &flag;
    bool const *bool_cp = bool_p;
    PV(bool_p); PVL(bool_cp);

    return 0;
}
