//
// Created by Rob Ross on 1/13/26.
//
// to make: clang substitution.c ../cs50.c
//

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../cs50.h"

void display_cli_args(int argc, char *argv[]) {
    // argv[0] is the name of the c program (entry point)
    printf("cl arguments: %d\n", argc);
    for (int arg_index = 0; arg_index < argc; arg_index++) {
        printf("arg %i. %s\n", arg_index, argv[arg_index]);
    }
}

bool key_is_valid(char * key) {
    //key must contain 26 alphabetic characters, each character may only appear once.  
    return true;
}

void display_key_set(char key_set[26]) {
    printf("key_set: \n");
    for (int i =0; i < 26; i++) {
        printf("%i, ", key_set[i]);
    }
    printf("\n");
}


// Populate the output array with the key set, return false if invalid
// every letter from A-Z (case-sensitive) must be present exactly once.
bool populate_key_set(char *key, char output[26]) {
    //Uppercase A = 65, lowercase a = 97, lowercase - uppercase = 32.
    // valid ranges are 65-90, 97-122
    int i = 0;
    int letter_count = 0;
    while ( key[i] != '\0') {
        if ( !( (key[i] >= 'A' && key[i] <='Z') || (key[i] >='a' && key[i] <= 'z')) ) {
            printf("Character %c is invalid.\n", key[i]);
            return false;
        }
        // key[i] is a valid character. Normalize it to upper-case for set comparisons.
        char key_char = toupper(key[i]);
        int key_char_index = key_char - 'A';
        if (output[key_char_index] != -1) {
            //we have already seen this character
            printf("output[%i] == %i: %i\n", key_char_index, true, (output[key_char_index] == true));
            printf("character %c already used. output[%d] = %d\n", key[i], key_char_index, output[key_char_index]);
            return false;
        }
        output[key_char_index] = i;
        letter_count++;
        i++;
        if (i > 26) {
            printf("Expected exactly 26 characters, input size exceeded.\n");
            return false;
        }
    }

    if (letter_count != 26) {
        printf("Expected 26 characters, got %d.\n", letter_count);
        return false;
    }

    return true;
}

void encrypt(char key_set[26], char *input,  char *output) {
    int index = 0;
    while (input[index] != '\0') {
        if (isalpha(input[index])) {
            char plain_char = toupper(input[index]);
            char cypher_char = key_set[ plain_char - 'A'] + 'A';
            // printf("plain_char: %c, cypher_char: %c, plain_char-'A': %i\n", plain_char, cypher_char, plain_char - 'A');
            output[index] = islower(input[index]) ? tolower(cypher_char): cypher_char;
        }
        else {
            output[index] = input[index];  // copy non-letters unchanged
        }
        index++;
    }
}

int main(int argc, char *argv[]) {

    display_cli_args(argc, argv);
    // argc includes the executable name.
    int num_args = argc - 1;
    if ( num_args != 1) {
        printf("invalid number of arguments. Expected 1, got %d\n", num_args);
        return 1;
    }
    printf("\nInput: %s\n", argv[1]);

    char key_set[26] = {0};
    memset(key_set, -1, sizeof(key_set));
    // display_key_set(key_set);

    if (!populate_key_set(argv[1], key_set)) {
        return 1;
    }

    // display_key_set(key_set);

    string plain_text = get_string("plaintext: ");
    // printf("plain_text: %s\n", plain_text);

    char cypher_text[strlen(plain_text)];
    encrypt(key_set, plain_text, cypher_text);

    printf("cyphertext: %s\n", cypher_text);
    return 0;

}