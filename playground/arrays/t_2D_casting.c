//
// Created by Rob Ross on 2/20/26.
//


#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../roblib/dev_utils.h"

void test_2D(size_t const sz) {
    int * grid_ptr = malloc(sz * sz * sizeof(int));
    if (!grid_ptr) return;

    int (*grid)[sz] = (int (*)[sz])grid_ptr;

    int sequence = 0;
    for (size_t row = 0; row < sz; ++row) {
        for (size_t col = 0; col < sz; ++col) {
            grid[row][col] = sequence++;
        }
    }

    du_repr_array_int_2D(sz, sz, grid);
    putchar('\n');
    free(grid_ptr);
}


// make:
// clang -std=c23 -Wall -Wextra -Wconversion -Werror -o t_2D_casting.out t_2D_casting.c ../../roblib/dev_utils.c

int main(void) {

    test_2D(3);
}
