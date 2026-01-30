//
// Created by Rob Ross on 1/28/26.
//



#ifndef POINTERS_H
#define POINTERS_H

#include <stdlib.h>

static inline void safe_free_ptr(void **pp)
{
    if (pp && *pp) {
        free(*pp);
        *pp = NULL;
    }
}

#define safe_free(PTR)   \
    do {                 \
        safe_free_ptr((void **)&(PTR)); \
    } while (0)

#endif //POINTERS_H
