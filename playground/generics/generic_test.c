#include <stdio.h>
#include <stdatomic.h>
int main() {
    int normal_int = 1;
    const int const_int = 2;
    volatile int volatile_int = 3;
    _Atomic int atomic_int = 4;
    printf("normal: %s\n", _Generic(normal_int, int: "int", default: "other"));
    printf("const: %s\n", _Generic(const_int, int: "int", default: "other"));
    printf("volatile: %s\n", _Generic(volatile_int, int: "int", default: "other"));
    printf("atomic: %s\n", _Generic(atomic_int, int: "int", default: "other"));
    return 0;
}
