#pragma once

#include <stddef.h>

typedef struct Stack {
    size_t count;
    size_t capacity;
    void **data;
} gc_stack_t;

gc_stack_t *stack_new(size_t capacity);

void stack_push(gc_stack_t *stack, void *obj);
void *stack_pop(gc_stack_t *stack);

void stack_free(gc_stack_t *stack);
void stack_remove_nulls(gc_stack_t *stack);
