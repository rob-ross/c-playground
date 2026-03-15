// auto_freeing.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/13 13:41:02 PDT

//
// Created by Rob Ross on 3/13/26.
//

#include <stdlib.h>
#include <stdio.h>
#include <_string.h>
#include <sys/_pthread/_pthread_mutex_t.h>

static void free_ptr(void *p)
{
    void **ptr = p;
    free(*ptr);
}


void t1(void)
{
    // NOTE: The cleanup function receives a pointer to the variable, not the variable itself.
    int *x __attribute__((cleanup(free_ptr))) = malloc(sizeof(int));

    *x = 42;
    printf("%d\n", *x);
}


static void close_file(FILE **f)
{
    if (*f)
        fclose(*f);
}

int t2(void)
{
    FILE *fp __attribute__((cleanup(close_file))) = fopen("test.txt", "r");

    if (!fp)
        return 1;

    /* use file */

}  // fclose automatically called

#define AUTOFREE __attribute__((cleanup(free_ptr)))

void t3(void) {
    AUTOFREE char *buffer = malloc(1024);

    //When the scope ends, buffer is freed automatically.
}

//A common trick is to write type-specific cleanup functions.
#define AUTO_CHAR __attribute__((cleanup(free_char)))
static void free_char(char **p)
{
    free(*p);
}

void t4(void) {
    AUTO_CHAR char *str = strdup("hello");

}

// defer:

#define defer(func) __attribute__((cleanup(func)))
void unlock(pthread_mutex_t **m)
{
    pthread_mutex_unlock(*m);
}

void f()
{
    pthread_mutex_t *m; // = ...;

    pthread_mutex_lock(m);
    defer(unlock) pthread_mutex_t *guard = m;

    /* critical section */

    //The mutex unlocks automatically when leaving scope.
}