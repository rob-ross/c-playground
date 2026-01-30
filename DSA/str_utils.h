//
// Created by Rob Ross on 1/27/26.
//

#ifndef STR_UTILS_H
#define STR_UTILS_H
#include <stdbool.h>
#include <stdio.h>

#define PVAR_I(X) printf(#X " is %i at address %p\n", X, &X);
#define PVAR_LU(X) printf(#X " is %lu at address %p\n", X, &X);

#define SWAPI(A, B) { int temp = a; a = b; b = temp; }

#define println(fmt, ...)                 \
    do {                                  \
        printf((fmt), ##__VA_ARGS__);     \
        putchar('\n');                    \
    } while (0)

char *concat(const char *str1, const char *str2);
const char *bstr(bool b);
char *itostr(int i) ;
char *empty_array_str(void);
char *astr(const int nums[], size_t nums_size);
void main_entry(void);

#endif //STR_UTILS_H