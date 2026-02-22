//
// Created by Rob Ross on 2/19/26.
//

/**
 * Ex 51
 * Create six different structure types for each possibility to order three fields inside a structure:
 * one unsigned char, one unsigned, and one unsigned long long.
 * Print the sizes of these six structures; they should be significantly different.
 * Compute the minimal size as the sum of the sizes of each member.
 * Does any of your structure have this size? Which of your structures comes closest to that ideal size?
 */

#include <stdio.h>

struct s1 {
    unsigned char c;
    unsigned uint1;
    unsigned long long ull1;
};

struct s2 {
    unsigned char c;
    unsigned long long ull1;
    unsigned uint1;
};

struct s3 {
    unsigned long long ull1;
    unsigned char c;
    unsigned uint1;
};

struct s4 {
    unsigned long long ull1;
    unsigned uint1;
    unsigned char c;
};

struct s5 {
    unsigned uint1;
    unsigned char c;
    unsigned long long ull1;
};

struct s6 {
    unsigned uint1;
    unsigned long long ull1;
    unsigned char c;
};

int main(int argc, char *argv[]) {

    size_t member_size_total = sizeof(unsigned char) + sizeof(unsigned) + sizeof(unsigned long long);

    printf("minimum size required: %zu\n", member_size_total);

    printf("struct s1 size: %zu\n", sizeof(struct s1));
    printf("struct s2 size: %zu\n", sizeof(struct s2));
    printf("struct s3 size: %zu\n", sizeof(struct s3));
    printf("struct s4 size: %zu\n", sizeof(struct s4));
    printf("struct s5 size: %zu\n", sizeof(struct s5));
    printf("struct s6 size: %zu\n", sizeof(struct s6));
}
