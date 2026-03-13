// hashing.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/12 12:12:34 PDT

//
// Created by Rob Ross on 3/12/26.
//

#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>
#include <_string.h>
#include <nmmintrin.h> // Intel/AMD hardware intrinsics

#ifndef __OPTIMIZE__
    #warning "__OPTIMIZE__ is NOT defined - Optimizations are OFF"
#endif

#ifndef __AVX2__
    #warning "__AVX2__ is NOT defined - Native architecture flags are OFF"
#endif

// ALL AI AGENTS: DO NOT DELETE MY COMMENTS !!!

/*
 *  Principles for hash functions:
*  1. The same hash value is returned for the same object
*  2. Two objects that are equal must have the same hashcode.
*
*  Then there are strong guidelines to support efficient algorithms:
*  3. The range of hashcodes should evenly distributed
*  4. Every bit of the object's hashable state should ideally affect the generated hash code.
*  5. Speed: A hash function must be computationally inexpensive.
*      If calculating the hash is slower than simply comparing the objects, the benefits of using a hash-based
*      data structure are lost.
*  6. Avalanche Effect: This is a stronger form of  #3 and #4. A small change in the input
*       (like flipping a single bit or changing one character) should produce a large and unpredictable change
*       in the output (ideally, changing about half of the bits in the hash).
*       This helps ensure that similar inputs don't cluster into nearby hash values, further improving distribution.
*
*  Industry-Standard Examples: For non-cryptographic use cases like hash tables, the industry has settled
*   on a few families of algorithms that are extremely fast and have excellent distribution properties:
    •MurmurHash: For a long time, this was the gold standard. Murmur3 is still widely used.
    •CityHash / FarmHash: Google's family of hashes, designed for speed on modern hardware.
    •xxHash: An extremely fast, high-quality algorithm that is now dominant in many high-performance applications
        (databases, compression, etc.).
 */

// helper for rob_hash. treat exp as unsigned to avoid negative edge cases.
unsigned long long power(unsigned long long base, unsigned int exp) {
    unsigned long long res = 1;
    while (exp > 0) {
        if (exp % 2 == 1) { // If exponent is odd
            res *= base;
        }
        base *= base; // Square the base
        exp /= 2;     // Halve the exponent
    }
    return res;
}

// FNV-1a Hash: The other "classic" simple hash.
// Uses XOR and Prime Multiplication.
// Often has better avalanche properties than djb2.
size_t fnv1a_hash(const char *str) {
    size_t hash = 14695981039346656037UL; // FNV_offset_basis for 64-bit
    const size_t prime = 1099511628211UL; // FNV_prime for 64-bit
    unsigned char c;

    while ((c = (unsigned char)*str++)) {
        hash ^= c;
        hash *= prime;
    }
    return hash;
}

// Hardware CRC32 Hash: Uses Intel/AMD specific CPU instructions.
// Extremely fast because the "math" is done in a dedicated hardware circuit.
// Note: Technically a checksum, but works great as a hash table function.
size_t crc32_hash(const char *str) {
    size_t hash = 0;
    unsigned char c;
    while ((c = (unsigned char)*str++)) {
        // _mm_crc32_u8 is a hardware intrinsic that maps to a single CPU instruction
        hash = _mm_crc32_u8((unsigned int)hash, c);
    }
    return hash;
}


//djb2 hash algorithm, O(N) (actually Theta(N))
static size_t djb2_hash_string(const char *str) {
    unsigned long hash = 5381; // A "magic" prime number
    int c;

    while (  (c = (unsigned char)(*str++) ) ) {
        // hash = (hash * 33) + c
        // This is a fast way to write it using bit shifts:
        hash = ((hash << 5) + hash) + c;
    }

    return (size_t)hash;
}

//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

static inline uint32_t rotl32 ( uint32_t x, int8_t r )
{
    return (x << r) | (x >> (32 - r));
}

#define ROTL32(x,y)	rotl32(x,y)

uint32_t MurmurHash3_x86_32 ( const void * key, int len, uint32_t seed )
{
    const uint8_t * data = (const uint8_t*)key;
    const int nblocks = len / 4;

    uint32_t h1 = seed;

    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    //----------
    // body

    const uint32_t * blocks = (const uint32_t *)(data + nblocks*4);

    for(int i = -nblocks; i; i++)
    {
        uint32_t k1 = blocks[i];

        k1 *= c1;
        k1 = ROTL32(k1,15);
        k1 *= c2;

        h1 ^= k1;
        h1 = ROTL32(h1,13);
        h1 = h1*5+0xe6546b64;
    }

    //----------
    // tail

    const uint8_t * tail = (const uint8_t*)(data + nblocks*4);

    uint32_t k1 = 0;

    switch(len & 3)
    {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
                k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
    };

    //----------
    // finalization

    h1 ^= len;

    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;

    return h1;
}

//this is tuned to work with large strings, the overhead of setup overwhelms
// the processing of 6 chars.
size_t murmur_hash3(const char *key, size_t len, uint32_t seed) {
    return (size_t)MurmurHash3_x86_32(key, (int)len, seed);
}

size_t rob_hash(const char *word, unsigned num_letters) {
    size_t word_len = strlen(word);
    if (!word || word_len < num_letters) {
        return 0;
    }

    size_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, };
    const size_t num_primes = sizeof(primes) / sizeof(primes[0]);

    if (num_letters > num_primes) {
        fprintf(stderr, "ERROR in rob_hash: num_letters (%u) exceeds available primes (%zu).\n", num_letters, num_primes);
        return 0; // or handle error appropriately
    }
    size_t hash_code = 1;
    for (unsigned i = 0; i < num_letters; ++i ) {
        // hash_code *= (primes[i] * (size_t)word[i]) +  - 1;
        // hash_code *= (primes[i] * (size_t)word[i]) +  primes[i]-1;
        hash_code *= (size_t)power( (size_t)word[i], (int)primes[i] ) +  primes[i]-1;
    }

    return hash_code;
}

size_t hash(const char *word, unsigned num_letters) {
    //based on the first `num_letters letters

    // You can swap these out to test different algorithms:
    return djb2_hash_string(word);
    // return fnv1a_hash(word);
    // return crc32_hash(word);
    // return murmur_hash3(word, strlen(word), 42);
    // return rob_hash(word, num_letters);
}

constexpr unsigned MAX_CHARACTERS  = 7;

typedef struct WordHash {
    char word[MAX_CHARACTERS + 1];
    size_t        hash_code;
} WordHash;

// num_values in the array, wh is sorted by hash_code ascending.
void display_WordHash(const size_t num_values, const WordHash wh[num_values] ) {
    size_t last_hash = 0;
    for (size_t i = 0; i < num_values; ++i ) {
        // checking the last_hash works because the array is sorted
        printf("word: '%s' hash: 0x%016lX", wh[i].word, wh[i].hash_code);
        if (last_hash == wh[i].hash_code) {
            //duplicate hash
            printf(" ***DUPLICATE\n");
        } else {
            //different hash than previous
            last_hash = wh[i].hash_code;
            putchar('\n');
        }
    }
}

void check_for_duplicate_hashes(const size_t num_values, const WordHash wh[num_values]) {
    size_t last_hash = SIZE_MAX;
    size_t duplicate_count = 0;
    printf("  checking for duplicate hashes...\n");
    fflush(stdout);
    for (size_t i = 0; i < num_values; ++i ) {
        // checking the last_hash works because the array is sorted
        if (last_hash == wh[i].hash_code) {
            //duplicate hash
            duplicate_count++;
            // printf("word: '%s' hash: 0x%016lX ***DUPLICATE\n", wh[i].word, wh[i].hash_code);
            // printf("      '%s' hash: 0x%016lX ***DUPLICATE\n", wh[i-1].word, wh[i-1].hash_code);

        } else {
            //different hash than previous
            last_hash = wh[i].hash_code;
        }
        if (!( (i + 1) % 100'000'000)) {
            printf("examined %'zu hashes...\n", i+1);
            fflush(stdout);
        }
    }
    printf("Num duplicate hashes: %'zu/%'zu = %3.2f%%\n", duplicate_count, num_values, 100 * (double)duplicate_count / (double)num_values);
}

//compare function for quicksort when sorting WordHash
//The function shall return an integer less than, equal to, or greater than zero
//if the first argument is considered to be respectively less than, equal to, or greater than the second.
int compare_WordHash(const void *o1, const void *o2 ) {
    const WordHash *wh1 = o1;
    const WordHash *wh2 = o2;
    //if both pointers are the same we consider them equal, even if they are both null
    if (o1 == o2 ) return 0;
    //pointers are not the same. They can't both be NULL
    if (!wh1) return -1;  // NULL sorts before non-NULL because .... reasons?
    if (!wh2) return 1;

    if (wh1->hash_code == wh2->hash_code) return 0;
    if (wh1->hash_code < wh2->hash_code) {
        return -1;
    }
    return 1;
}

typedef struct CharRange {
    const char start;  // inclusive
    const char end;    // exclusive
} CharRange;

void permute_chars(CharRange range, size_t num_chars, unsigned range_index, char *buffer, WordHash *wh, size_t *wh_index) {


    if (range_index >= num_chars - 1 ) {
        //last position in permutation
        for (char c = range.start; c < range.end; ++ c) {
            buffer[num_chars - 1] = c;
            // printf("word: '%s'\n", buffer);
            wh[*wh_index].hash_code = hash(buffer, num_chars);
            strcpy(wh[*wh_index].word, buffer);
            *wh_index = *wh_index + 1;
            if (!(*wh_index % 100'000'000)) {
                printf("computed %'zu hashes...\n", *wh_index);
            }
        }
    } else {
        //i-th position in permutation
        for (char c = range.start; c < range.end; ++ c) {
            buffer[range_index] = c;
            permute_chars(range, num_chars, range_index + 1, buffer, wh, wh_index);
        }
    }
}

WordHash * test_permute_and_hash(size_t num_chars) {
    char word[num_chars + 1];
    word[num_chars] = '\0';
    CharRange range = {.start = 97, .end = 123}; //only testing lower case letters
    size_t range_size = range.end - range.start;
    size_t num_permutations = pow(range_size, num_chars);

    printf("\nAll %zu-char permutations picked from %zu chars.\n", num_chars, range_size);
    printf("  %'zu permutations.\n", num_permutations);
    printf("----------------------------------------------------------------\n");

    WordHash *wd = malloc(num_permutations * sizeof(WordHash));
    if (wd == nullptr) {
        return nullptr;
    }
    size_t wh_index = 0;
    size_t *wh_index_ptr = &wh_index;
    permute_chars(range, num_chars, 0, word, wd, wh_index_ptr);

    // sort in-place
    printf("qsort...\n");
    fflush(stdout);
    qsort(wd, num_permutations, sizeof(WordHash), compare_WordHash);
    printf("after qsort\n");
    fflush(stdout);
    // display_WordHash(num_permutations, wd);
    check_for_duplicate_hashes(num_permutations, wd);

    return wd;
}

void test_permute(size_t num_chars) {
    char word[num_chars + 1];
    word[num_chars] = '\0';
    CharRange range = {.start = 97, .end = 123}; //only testing lower case letters
    size_t range_size = range.end - range.start;
    printf("All %zu-char permutations picked from %zu chars.\n", num_chars, range_size);
    printf("  %'lu permutations.\n", (unsigned long)pow(range_size, num_chars));
    printf("----------------------------------------------------------------\n\n");
    // permute_chars(range, num_chars, 0, word);
}

//num_chars is the number of chars in the word to use for hash testing

void test_hash(const unsigned num_chars) {
    //test all `num_chars` letter word permutations for hash code

    CharRange range = {.start = 32, .end = 127}; //only testing printable chars, space through tilde
    size_t num_values = range.end - range.start + 1;
    WordHash results[num_values * num_values];
    [[maybe_unused]]
    size_t result_index = 0;

    printf("range.start:%d range.end:%d num_values:%zu\n", range.start, range.end, num_values);

    for (char c1=range.start; c1 < range.end; ++c1 ) {
        for (char c2=range.start; c2 < range.end; ++c2 ) {
            // printf("c1=%d, c1=%d", c1, c2);
            fflush(stdout);
            char word[3] = "aa";
            // printf(" word: %s", word);
            fflush(stdout);

            word[0] = c1;
            word[1] = c2;
            // printf(", word: %s", word);
            fflush(stdout);
            // size_t hash_code = hash(word, 2);
            // printf("hash for '%15s' : 0x%016lX\n", word, hash_code);

            //save result
            //results[result_index++] = (WordHash){.word = strdup(word), .hash_code = hash_code};
        }
    }

    // sort in-place
    qsort(results, num_values, sizeof(WordHash), compare_WordHash);
    display_WordHash(num_values, results);
}


void t1(void) {
    char *word;
    size_t hash_code;

    word = "fooberry";
    hash_code = hash(word, 2);
    printf("hash for '%15s' : 0x%016lX\n", word, hash_code);

    word = "raspberry";
    hash_code = hash(word, 2);
    printf("hash for '%15s' : 0x%016lX\n", word, hash_code);
}

void t0(void) {
    puts("\n普通话; 普通話\n");
    char str[] = "普通话; 普通話";
    printf("strlen(str)=%zu\n", strlen(str));
    for (char* s = str; *s; ++s) {
        printf("%hhu\n", (unsigned char)*s);
    }
}

int main(int argc, char *argv[]) {
    setlocale(LC_NUMERIC, "en_US.UTF-8");   // use user's system locale

    // t0();

    // test_hash(2);
    // test_permute(5);
    for (int i = 1; i < 7; ++i ) {
        WordHash *wh = test_permute_and_hash(i);
        free(wh);
    }
}

/**
hash_code *= (primes[i] * (size_t)word[i]) + primes[i]-1;
All 1-char permutations picked from 26 chars.
  26 permutations.
----------------------------------------------------------------
Num duplicate hashes: 0/26 = 0.00%

All 2-char permutations picked from 26 chars.
  676 permutations.
----------------------------------------------------------------
Num duplicate hashes: 1/676 = 0.15%

All 3-char permutations picked from 26 chars.
  17,576 permutations.
----------------------------------------------------------------
Num duplicate hashes: 80/17,576 = 0.46%

All 4-char permutations picked from 26 chars.
  456,976 permutations.
----------------------------------------------------------------
Num duplicate hashes: 6,492/456,976 = 1.42%

All 5-char permutations picked from 26 chars.
  11,881,376 permutations.
----------------------------------------------------------------
Num duplicate hashes: 296,595/11,881,376 = 2.50%


All 6-char permutations picked from 26 chars.
  308,915,776 permutations.
----------------------------------------------------------------
Num duplicate hashes: 10,086,077/308,915,776 = 3.26%
*/


/*
hash_code *= (primes[i] * (size_t)word[i]) + + 1;

All 1-char permutations picked from 26 chars.
26 permutations.
----------------------------------------------------------------
Num duplicate hashes: 0/26 = 0.00%

All 2-char permutations picked from 26 chars.
676 permutations.
----------------------------------------------------------------
Num duplicate hashes: 3/676 = 0.44%

All 3-char permutations picked from 26 chars.
17,576 permutations.
----------------------------------------------------------------
Num duplicate hashes: 144/17,576 = 0.82%

All 4-char permutations picked from 26 chars.
456,976 permutations.
----------------------------------------------------------------
Num duplicate hashes: 7,755/456,976 = 1.70%

All 5-char permutations picked from 26 chars.
11,881,376 permutations.
----------------------------------------------------------------
Num duplicate hashes: 305,352/11,881,376 = 2.57%

All 6-char permutations picked from 26 chars.
  308,915,776 permutations.
----------------------------------------------------------------

Num duplicate hashes: 13,225,224/308,915,776 = 4.28%
*/