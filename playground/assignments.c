//
// Created by Rob Ross on 1/31/26.
//

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Can we cast a string to an char[] in order to copy the contents to a new string pointer?
 */
void test_array_cast(void) {
    char *a_str = "This is the string to copy";
    size_t a_str_len = strlen(a_str);

    char *copy_of_a_str = malloc(a_str_len);
    if (!copy_of_a_str) {
        exit(1);
    }

    // *copy_of_a_str = (char[a_str_len])a_str;  // ?? is this possible?  nope


    char char_array1[] = "This is the string to copy";
    size_t char_array1_len = strlen(char_array1);
    // char char_array1_copy[char_array1_len] = (char[char_array1_len])char_array1;  // nope


    char *str2 = "I'm a string literal!";
    size_t str2_len = strlen(str2);

    // char char_array2[] = (char[])str2;  // nope

    // Maybe fixed size arrays will work?
    // char *str3 = "fixed"; // size == 5

    // char str3_array[5] = (char[5])str3;  // nope

}

struct Struct1 {
    int int1;
    int int_array1[5];
};

struct Struct2 {
    int int1;
    char char_array[20];
};

void display_struct1(struct Struct1 *s) {
    printf("Struct1(%i, [", s->int1);
    for (int i = 0; i < 5; ++i) {
        printf("%i, ", s->int_array1[i]);
    }
    printf("])\n");
}

void display_struct2(struct Struct2 *s) {
    printf("Struct2(%i, [", s->int1);
    for (int i = 0; i < 20; ++i) {
        printf("%c, ", s->char_array[i]);
    }
    printf("])\n");
}

void test_struct_assignment(void) {
    // arrays wrapped in a struct are copied when the struct is copied!

    struct Struct1 struct1 = { .int1 = 42, .int_array1 = {1,2,3}};
    printf("struct1: ");
    display_struct1(&struct1);
    struct Struct1 struct2 = struct1;
    printf("struct2:\n");
    display_struct1(&struct2);


    // fewer items than the buffer size, \0 is written after H and the rest of the buffer is zeroed.
    struct Struct2 s2 = {1, "H"};
    struct Struct2 s3;
    printf("s2: ");
    display_struct2(&s2);
    printf("s3: ");
    display_struct2(&s3);

    printf("after copy:\n");
    s3 = s2;
    display_struct2(&s3);
}

void legit_array_assignment(void *dest, const void *source, size_t size) {
    //copy contents of source array into destination array
    memcpy(dest, source, size);

}

void display_string(const char *char_ptr) {
    const size_t len = strlen(char_ptr);
    printf("string len=%zu : '",len);
    for (size_t i = 0; i < len; ++i) {
        printf("%c",char_ptr[i]);
    }
    printf("'\n");
}

void test_legit_array_assignment(void) {
    char a1[10 + 1] = "short text";  // 10 characters + terminator
    char a2[10 + 1] = {0};

    display_string(a1);
    display_string(a2);

    legit_array_assignment(a2, a1, strlen(a1));
    display_string(a2);

}

int main(void) {
    // test_struct_assignment();
    test_legit_array_assignment();

    return 0;
}