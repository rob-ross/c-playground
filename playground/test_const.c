#include <stdio.h>
#include <stdbool.h>
bool strings_same(const char *s1, const char *s2) { return s1 == s2; }
int main() {
 char str1[] = "hello";
 char *p1 = str1;
 if (strings_same(p1, p1)) printf("Success\n");
 return 0;
}