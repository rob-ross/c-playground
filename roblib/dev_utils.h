//
// Created by Rob Ross on 2/20/26.
//

#pragma once
#include <stddef.h>


// expands to printf( fmt, ...)  but adds a newline to the end of fmt.
#define print(fmt, ...)                 \
    do {                                \
        printf((fmt), ##__VA_ARGS__);   \
        putchar('\n');                  \
    } while (0)


void repr_array_int(size_t const n, const int array[static n]);
void du_repr_array_int_2D(size_t const rows, size_t const cols, int const array[static rows][ cols]);