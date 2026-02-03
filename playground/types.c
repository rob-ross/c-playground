//
// Created by Rob Ross on 1/6/26.
//

#include <stdint.h>
#include <stdio.h>

void std_int_types(void) {
    typedef int8_t byte;

    //regular signed ints
    // ReSharper disable CppLocalVariableMayBeConst
    byte b = 127;
    short int si = -10;
    int i = -1;
    long int li = 100;
    long long int lli = 1000;

    printf("byte b = %hhd\n", b);
    printf("short int si = %hd\n", si);
    printf("int i = %d\n", i);
    printf("long int li = %ld\n", li);
    printf("long long int lli = %lld\n", lli);

    //unsigned ints, "unsigned", "signed" are 'qualifiers')
    typedef uint8_t ubyte;
    ubyte ub = 255;
    unsigned short int usi = 10;
    unsigned int ui = 1;
    unsigned long int uli = 100;
    unsigned long long int ulli = 1000;

    printf("\n");
    printf("unsigned ubyte ub = %hhu\n", ub);
    printf("unsigned short int usi = %hu\n", usi);
    printf("unsigned int ui = %u\n", ui);
    printf("unsigned long int uli = %lu\n", uli);
    printf("unsigned long long int ulli = %llu\n", ulli);

    byte sb = -127;
    printf("\nsigned byte -127 as unsigned byte= %hhu\n", sb);
    printf("unsigned ubyte 255 as signed byte = %hhd\n", ub);


}

void float_types(void) {
    float  f = 42.42;
    double d = 42.42;
    long double ld = 42.42;  // may have same length as double.

    printf("\nsizeof float: %lu, sizeof double: %lu, sizeof long double: %lu\n", sizeof(f), sizeof(d), sizeof(ld));
}

void char_types(void) {
    // Single byte (Legacy Extended ASCII) - often fails in modern terminals
    unsigned char legacy = 163;
    printf("\nLegacy Byte: %c (Value: %hhu)\n", legacy, legacy);

    // UTF-8 String (Modern way) - works in CLion's default settings
    // The £ symbol is actually TWO bytes in UTF-8: 0xC2 0xA3
    char *modern = "£";
    printf("Modern UTF-8: %s\n", modern);

    char c = 'c';
    signed char sc = -128;
    unsigned char uc = 169;

    printf("char c = %c\n", c);
    printf("signed char c = %c\n", sc);
    printf("unsigned char as int byte uc = %hhu\n", uc);
    printf("unsigned char as char uc = %c\n", uc);

}



int main (void) {
    std_int_types();
    char_types();
    float_types();
}