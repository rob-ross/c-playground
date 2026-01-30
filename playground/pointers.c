//
// Created by Rob Ross on 1/28/26.
//
// make: clang pointers.c -o pointers

#include <signal.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#include "pointers.h"

static sigjmp_buf env;

void segv_handler(int sig) {
    (void)sig;
    // WARNING: only async-signal-safe functions are allowed here
    siglongjmp(env, 1);
}

void int_pointers(void) {

    // a pointer is a memory address. More specifically, a pointer is a *variable*, whose *value* is a memory address.

    int *i1_ptr;  // i1_ptr is a pointer to an int.
    // here, i1_ptr has not been initialized, so it contains a garbage value.
    // there is no storage created for an int for i1_ptr to point to.

    char *format_string = "  int *i1_ptr: pointer value i1_ptr= %p (garbage), deferenced value *i1_ptr = %i (garbage)\n";

    printf("i1_ptr uninitialized: \n");
    printf( format_string, i1_ptr, *i1_ptr);

    // it's safer to initialize new pointers to NULL if you don't assign them an explicit value immediately:
    int *i2_ptr = NULL;

    printf("\ni2_ptr initialized to NULL: \n");
    printf("  int *i2_ptr: pointer value i2_ptr= %p (NULL)\n", i2_ptr);

    struct sigaction sa = {0};
    sa.sa_handler = segv_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, NULL);

    if (sigsetjmp(env, 1) == 0) {
        printf("\nAbout to dereference NULL pointer *i2_ptr...\n");
        int garbage = *i2_ptr;
        printf("  You will not see this.\n");
    } else {
        printf("  Recovered from SIGSEGV (unsafe, demo only).\n");
    }

    printf("Continuing program...\n");

    printf("\nNow we have an int variable created and initialized, and a pointer value initialized to point to this int.\n");
    int i3 = 42;
    int *i3_ptr = &i3;

    printf("  i3 = %i, &i3 = %p, i3_ptr = %p, *i3_ptr = %i\n", i3, &i3, i3_ptr, *i3_ptr);

    printf("\nNow we will dynamically allocate an int on the heap with malloc.\n");
    int *i4_ptr = malloc(sizeof(int));
    if (i4_ptr == NULL) {
        printf("  malloc returned NULL for malloc(sizeof(int)); ");
    } else {
        printf("  malloc returned :%p, i4_ptr = %p, *i4_ptr = %i (garbage)\n", i4_ptr, i4_ptr, *i4_ptr);
        *i4_ptr = 42;
        printf("   called *i4_ptr = 42, i4_ptr = %p, *i4_ptr = %i\n", i4_ptr, *i4_ptr);
        safe_free(i4_ptr);
        printf("  called free(i4_ptr);, setting i4_ptr to NULL. i4_ptr = %p\n", i4_ptr);
    }

}

void int_arrays(void) {

    // arr is an array of int.
    int arr[] = { 1,2,3,4,5 };
    printf("int arr[] = { 1,2,3,4,5 };\n");
    printf("  arr = %p. 'arr' is a pointer!   sizeof(arr) = %lu, sizeof(&arr) = %lu  (void*)&arr=%p\n", arr, sizeof(arr),  sizeof(&arr), (void*)&arr);
    printf("  *arr = %i. 'arr' points to the first element of the array 'arr'. \n", *arr);
    printf("  *( arr + 1 ) = %i. This is the second element of the array at index 1.\n", *(arr + 1));
    printf("    This is equivalent to arr[1] = %i\n", arr[1]);
    printf("&arr[0] = %p\n", &arr[0]);
    printf("&arr[1] = %p\n", &arr[1]);
    printf("**&arr = %i. &arr is address of array. *&arr is the value of the entire 5 element array. **&arr is value of first element.\n", **&arr);

    // arrays live on the stack.
    // with arr defined as above:
    // arr[0] == 1; // first element of arr
    // arr[1] == 2; // second element of arr
    // *arr == 1; // we can dereference the name of the array and it acts like a pointer to the first element
    // *(arr + 1) == 2; // using pointer arithmetic. same as arr[1]

}


int main(void) {

    // int_pointers();
    int_arrays();
}