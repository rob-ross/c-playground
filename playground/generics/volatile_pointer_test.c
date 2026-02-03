#include <stdio.h>
int main() {
    int i = 0;
    int *p = &i;
    volatile int *vp = &i;
    const volatile int *cvp = &i;
    printf("int*: %s\n", _Generic(p, int*: "match", default: "no match"));
    printf("volatile int*: %s\n", _Generic(vp, volatile int*: "match", default: "no match"));
    printf("const volatile int*: %s\n", _Generic(cvp, const volatile int*: "match", default: "no match"));
    return 0;
}
