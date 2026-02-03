#include <stdio.h>
int main() {
    int i = 0;
    int *p = &i;
    const int *cp = &i;
    int * const pc = &i;
    printf("int*: %s\n", _Generic(p, int*: "match", default: "no match"));
    printf("const int*: %s\n", _Generic(cp, const int*: "match", default: "no match"));
    printf("int* const: %s\n", _Generic(pc, int*: "match", default: "no match"));
    return 0;
}
