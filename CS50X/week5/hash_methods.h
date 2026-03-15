// hash_methods.h
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/13 12:54:03 PDT

//
// Created by Rob Ross on 3/13/26.
//

#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct CharRange {
    char start;  // inclusive
    char end;    // exclusive
} CharRange;

unsigned long long power(unsigned long long base, unsigned int exp);

size_t fnv1a_hash(const char *str, unsigned num_letters);

// crc32_hash is fastest when native crc function is available
size_t crc32_hash(const char *str, unsigned num_letters);

// second fastest, behind crc32_hash. Best non-optimized. general-purpose method
size_t djb2_hash_string(const char *str, unsigned num_letters);

size_t murmur_hash3(const char *key, unsigned len);
size_t rob_hash(const char *word, unsigned num_letters);

//hash function pointer
//num_chars is the number of chars in the word to use for hashing. Pass strlen(word) to include all characters
typedef size_t (*HashFunc)(const char *word, unsigned num_chars) ;