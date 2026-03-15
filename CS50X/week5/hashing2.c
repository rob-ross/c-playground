// hashing2.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/13 12:52:02 PDT

//
// Testing performance of various hashing methods for strings
//

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_methods.h"

#include "../../roblib/dev_utils.h"

typedef struct PermuteParams {
    const HashFunc hash_func;
    const size_t num_chars;
    char * const buffer;
    const int  range_start;
    const int  range_end;
} PermuteParams;

static PermuteParams params;

static size_t wh_index_global;


//tracking distribution of hash function output
// bucket count, bucket array[bucket count],
// bucket index function : hash & (m -1) if power of 2, else hash % m if prime.
// metric : reduced chi-square
struct HashDistribution {
    unsigned num_buckets;
    unsigned max_bucket_count; // most number of hashes to a bucket
    unsigned min_bucket_count; // least number of hashes to a bucket
    double   mean_bucket_count; // average number of hashes to all buckets
    //reduced chi-squares value of bucket index distribution.
    // close to 1.0 : ideal random-like distribution
    // much larger than 1.0: worse than expected, uneven
    // much smaller than 1.0: suspiciously "too smooth" (usually indicates falsified data)

    double   red_chi_squares;
    bool is_powerof2;

    size_t bucket_counts[]; // Flexible array.
};



unsigned calc_bucket_index(size_t hash_code, unsigned num_buckets, bool is_powerof2) {
    if (is_powerof2) {
        return hash_code & ( num_buckets -1 );
    } else {
        return hash_code % num_buckets;
    }
}

// state used by permute_chars_and_hash that does not change during recursive descent.
// We keep them in a global variable so they don't have to be repeatedly copied on the next stack frame.
// they must be set before the initial call to permute_chars_and_hash
void permute_chars_and_hash2(unsigned range_index) {

    if (range_index >= params.num_chars - 1 ) {
        //last position in permutation
        for (int c = params.range_start; c < params.range_end; ++ c) {
            params.buffer[params.num_chars - 1] = c;
            // printf("word: '%s'\n", buffer);
            size_t result = params.hash_func(params.buffer, params.num_chars);
            wh_index_global = wh_index_global + 1;
            if (wh_index_global == 1'000'000'000 ) {
                printf("\n");
            }
            if (!(wh_index_global % 1'000'000'000)) {
                printf("computed %'zu hashes...\n", wh_index_global);
            }
        }
    } else {
        //i-th position in permutation
        for (int c = params.range_start; c < params.range_end; ++ c) {
            params.buffer[range_index] = c;
            permute_chars_and_hash2(range_index + 1);
        }
    }
}

void hash_all_permutations_with2(const HashFunc func, const unsigned num_chars) {
    char word[num_chars + 1];
    word[num_chars] = '\0';
    CharRange range = {.start = 97, .end = 123}; //only testing lower case letters
    size_t range_size = range.end - range.start;
    size_t num_permutations = power(range_size, num_chars);

    printf("All %u-char permutations picked from %zu chars. ", num_chars, range_size);
    printf("  %'12zu permutations. ", num_permutations);
    //set up function's parameters
    PermuteParams temp = {.hash_func = func, .num_chars = num_chars, .range_start = range.start, .range_end = range.end, .buffer = word};
    memcpy(&params, &temp, sizeof(PermuteParams));
    // params.hash_func = func;
    // params.num_chars = num_chars;
    // params.wh_index = 0;
    // params.range = range;
    // params.buffer = word;
    wh_index_global = 0;
    permute_chars_and_hash2( 0);
    printf("  Hashed %'zu words. ", wh_index_global);
}

//we permute all characters in the CharRange for a string of length num_chars, and call the hash function in the func
// argument. This is intended to be a timing test, so we don't store the hash or check for collisions, just
// call the hash function for the string.
void permute_chars_and_hash(HashFunc hash_func, CharRange range, size_t num_chars, unsigned range_index, char *buffer, size_t *wh_index) {

    if (range_index >= num_chars - 1 ) {
        //last position in permutation
        for (char c = range.start; c < range.end; ++ c) {
            buffer[num_chars - 1] = c;
            // printf("word: '%s'\n", buffer);
            size_t result = hash_func(buffer, num_chars);
            *wh_index = *wh_index + 1;
            if (*wh_index == 1'000'000'000 ) {
                printf("\n");
            }
            if (!(*wh_index % 1'000'000'000)) {
                printf("computed %'zu hashes...\n", *wh_index);
            }
        }
    } else {
        //i-th position in permutation
        for (char c = range.start; c < range.end; ++ c) {
            buffer[range_index] = c;
            permute_chars_and_hash(hash_func, range, num_chars, range_index + 1, buffer, wh_index);
        }
    }
}



//instead of creating dummy strings, most of which are not actual words that would ever be hashed,
// we can use the wiktionary word data and assemble a list of actual English words of various sizes and
// use those for more realistic testing scenarios.

void hash_all_permutations_with(HashFunc func, unsigned num_chars) {
    char word[num_chars + 1];
    word[num_chars] = '\0';
    CharRange range = {.start = 97, .end = 123}; //only testing lower case letters
    size_t range_size = range.end - range.start;
    size_t num_permutations = power(range_size, num_chars);

    printf("All %u-char permutations picked from %zu chars. ", num_chars, range_size);
    printf("  %'12zu permutations. ", num_permutations);
    size_t wh_index = 0;
    size_t *wh_index_ptr = &wh_index;
    permute_chars_and_hash(func, range, num_chars, 0, word, wh_index_ptr);
    printf("  Hashed %'zu words. ", wh_index);
}

void time_all_hash_methods(void) {
    // Macro to pair the function pointer with its string name
    #define HASH_ENTRY(f) { f, #f }

    struct {
        HashFunc func;
        const char *name;
    } funcs[] = {
        HASH_ENTRY(fnv1a_hash),
        HASH_ENTRY(crc32_hash),
        HASH_ENTRY(djb2_hash_string),
        HASH_ENTRY(murmur_hash3),
        HASH_ENTRY(rob_hash)
    };

    constexpr unsigned num_funcs = sizeof(funcs) / sizeof(funcs[0]);

    for (unsigned f = 0; f < num_funcs; ++f ) {
        printf("\n\nTesting function: %s\n", funcs[f].name);
        printf("----------------------------------------------------------------------\n");
        double total_elapsed = 0;
        for (int i = 1; i < 9; ++i ) {
            total_elapsed += TIMEIT(hash_all_permutations_with2(funcs[f].func, i));
            printf("\n");
        }
        printf("Total elapsed time: %.6f s\n", total_elapsed);
    }
}


// make:

//DEBUG:
//  clang -std=c23 -o ./hashing2.out hashing2.c hash_methods.c ../../roblib/dev_utils.c
//
//RELEASE:
// clang -std=c23 -O3 -march=native -DNDEBUG -o ./hashing2.out hashing2.c hash_methods.c ../../roblib/dev_utils.c

int main(int argc, char *argv[]) {
    setlocale(LC_NUMERIC, "en_US.UTF-8");   // use user's system locale

    time_all_hash_methods();

}
