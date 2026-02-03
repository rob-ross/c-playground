#include <stdio.h>
int main() {
char c = 'a';
signed char sc = 'b';
unsigned char uc = 'c';
printf("char: %s\n", _Generic(c, char: "char", signed char: "signed char", unsigned char: "unsigned char", default: "other"));
printf("signed char: %s\n", _Generic(sc, char: "char", signed char: "signed char", unsigned char: "unsigned char", default: "other"));
printf("unsigned char: %s\n", _Generic(uc, char: "char", signed char: "signed char", unsigned char: "unsigned char", default: "other"));
return 0;
}
