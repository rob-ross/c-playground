// hash_methods.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/13 12:54:03 PDT

//
// Created by Rob Ross on 3/13/26.
//

#include "hash_methods.h"

#include <crc32intrin.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


#if defined(NDEBUG) &&  !defined(__OPTIMIZE__)
    #warning "__OPTIMIZE__ is NOT defined - Optimizations are OFF"
#endif

#if defined(NDEBUG) && !defined(__AVX2__)
    #warning "__AVX2__ is NOT defined - Native architecture flags are OFF"
#endif


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
size_t fnv1a_hash(const char *str, unsigned num_letters) {
    size_t hash = 14695981039346656037UL; // FNV_offset_basis for 64-bit
    const size_t prime = 1099511628211UL; // FNV_prime for 64-bit
    unsigned char c;

    while ((c = (unsigned char)*str++)) {
        hash ^= c;
        hash *= prime;
    }
    return hash;
}

// Fastest Hash function when compiled with -O3 -march=native
// Hardware CRC32 Hash: Uses Intel/AMD specific CPU instructions.
// Extremely fast because the "math" is done in a dedicated hardware circuit.
// Note: Technically a checksum, but works great as a hash table function.
__attribute__((target("crc32")))
inline size_t crc32_hash(const char *str, unsigned num_letters) {
    size_t hash = 0;
    unsigned char c;
    while ((c = (unsigned char)*str++)) {
        // _mm_crc32_u8 is a hardware intrinsic that maps to a single CPU instruction
        hash = _mm_crc32_u8((unsigned int)hash, c);
    }
    return hash;
}


//djb2 hash algorithm, O(N) (actually Theta(N))
size_t djb2_hash_string(const char *str, unsigned num_letters) {
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
size_t murmur_hash3(const char *key, unsigned num_letters) {
    return (size_t)MurmurHash3_x86_32(key, (int)num_letters, 42);
}

size_t rob_hash(const char *word, const unsigned num_letters) {

    constexpr size_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, };  // we can expand this array if needed
    constexpr size_t num_primes = sizeof(primes) / sizeof(primes[0]);

    // if (num_letters > num_primes) {
    //     fprintf(stderr, "ERROR in rob_hash: num_letters (%u) exceeds available primes (%zu).\n", num_letters, num_primes);
    //     return 0; // or handle error appropriately
    // }


    size_t hash_code = 1;
    for (unsigned i = 0; i < num_letters; ++i ) {
        // hash_code *= (primes[i] * (size_t)word[i]) +  - 1;
        // hash_code *= (primes[i] * (size_t)word[i]) +  primes[i]-1;
        hash_code *= (size_t)power( (size_t)word[i], (int)primes[i] ) +  primes[i]-1;
    }

    return hash_code;
}