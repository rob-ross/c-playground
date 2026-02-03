#include <stdio.h>
int main() {
int x = 5;
const char *fmt = _Generic(x,
int: "int",

float: "float",
default: "other"
);
printf("%s\n", fmt);
return 0;
}
