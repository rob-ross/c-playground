#pragma once

#include <stddef.h>

typedef struct Stack {
    size_t count;
    size_t capacity;
    void **data;
} snekstack_t;

snekstack_t *stack_new(size_t capacity);
void stack_push(snekstack_t *stack, void *obj);
void *stack_pop(snekstack_t *stack);
void stack_free(snekstack_t *stack);
