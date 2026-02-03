
#include "snekstack.h"
#include <stddef.h>
#include <stdlib.h>

void stack_free(snekstack_t *stack) {
    if (stack == NULL) {
        return;
    }

    if (stack->data != NULL) {
        free(stack->data);
    }

    free(stack);
}

void *stack_pop(snekstack_t *stack) {
    if (stack->count == 0) {
        return NULL;
    }

    stack->count--;
    return stack->data[stack->count];
}

void stack_push(snekstack_t *stack, void *obj) {
    if (stack->count == stack->capacity) {
        stack->capacity *= 2;
        void **temp = realloc(stack->data, stack->capacity * sizeof(void *));
        if (temp == NULL) {
            stack->capacity /= 2;
            exit(1);
        }
        stack->data = temp;
    }
    stack->data[stack->count] = obj;
    stack->count++;
    return;
}

snekstack_t *stack_new(size_t capacity) {
    snekstack_t *stack = malloc(sizeof(snekstack_t));
    if (stack == NULL) {
        return NULL;
    }

    stack->count = 0;
    stack->capacity = capacity;
    stack->data = malloc(stack->capacity * sizeof(void *));
    if (stack->data == NULL) {
        free(stack);
        return NULL;
    }

    return stack;
}
