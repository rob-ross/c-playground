//
// Created by Rob Ross on 1/5/26.
//

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "../cs50.h"

#define MAX_PYRAMID_HEIGHT 23
#define PYRAMID_H_GAP 2

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

void meow_forever(void) {
    // ReSharper disable once CppDFAEndlessLoop
    while (true) {
        printf("meow\n");
        // To sleep for 100ms (100,000 microseconds)
        usleep(100000);  // to allow time to process a cntl-c
    }
}

void meow_n(int n) {
    for (int i = 0; i < n; i++) {
        printf("meow\n");
        // To sleep for 100ms (100,000 microseconds)
        usleep(100000);
    }
}

int ask_for_n() {
    return get_int("What's n? ");
}

/* Prints a tower pyramid of hash symbols to the console of height `num_rows` 
 *
 */
void print_hash_pyramid(int num_rows) {
    num_rows = min(num_rows, MAX_PYRAMID_HEIGHT);
    for (int row = 1; row <= num_rows ; row++){
        int leading_spaces = num_rows - row;
        // left spaces
        for (int space = 0; space < leading_spaces; space++) {
            printf(" ");
        }
        // left side of pyramid
        for (int col = 1; col <= row; col++) {
            printf("#");
        }
        // pyramid gap
        for (int space = 0; space < PYRAMID_H_GAP; space++) {
            printf(" ");
        }
        // right side of pyramid
        for (int col = 1; col <= row; col++) {
            printf("#");
        }

        printf("\n");
    }
}

void print_hash_pyramid2(int num_rows) {
    num_rows = min(num_rows, MAX_PYRAMID_HEIGHT);
    for (int row = 1; row <= num_rows ; row++) {
        const int num_spaces = num_rows - row;
        int num_hashes = row;
        const int line_len = num_spaces + num_hashes * 2 + PYRAMID_H_GAP + 1 ;
        char line[num_spaces + num_hashes * 2 + PYRAMID_H_GAP + 1 ];  // +1 for the null terminator '\0'

        int current_pos = 0;
        memset(line, ' ', num_spaces);
        current_pos += num_spaces;
        memset(line + current_pos, '#', num_hashes);
        current_pos += num_hashes;
        memset(line + current_pos, ' ', PYRAMID_H_GAP);
        current_pos += PYRAMID_H_GAP;
        memset(line + current_pos, '#', num_hashes);

        line[line_len - 1] = '\0';
        printf("%s\n", line);
    }

}


int main(void) {
    //meow_n(ask_for_n());
    print_hash_pyramid2(5);
}