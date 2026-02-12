#include <stdio.h>
#include <locale.h>
int main() { if (setlocale(LC_ALL, "en_US.UTF-8")) { printf("%'d\n", 1234567); } else { printf("Locale not supported\n"); } return 0; }
