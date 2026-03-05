// string_interner.h
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/03 15:47:03 PST


#pragma once
#include "hashmap.h"

// Forward-declare the struct to make it an opaque type.
// Users can only have pointers to it, not instances.
// Full definition in string_interner.c
struct InternStringMap;
typedef struct InternStringMap InternStringMap;


// ---------------------------
// Public API methods
// ---------------------------

InternStringMap * instr_create(size_t num_buckets);
void instr_clear(InternStringMap *ismap);
bool instr_contains_key(InternStringMap *ismap, const char* strkey);
void instr_destroy(InternStringMap *ismap);

long instr_get_count(const InternStringMap *ismap, const char *key);
const char* instr_intern(InternStringMap *string_pool, char string[static 1]);
bool instr_is_empty(InternStringMap *ismap);
void instr_put(InternStringMap *ismap, const char* key) ;

void instr_remove(InternStringMap *ismap, const char* strkey);
size_t instr_size(InternStringMap *ismap);

void instr_repr_InternStringMap(InternStringMap *ismap, bool verbose);

/*
// In your application code...

// 1. Create the dependency (the string interner)
StringInterner* interner = string_interner_create();

// 2. Define the policy functions that adapt the interner to the policy interface
MapValue intern_policy_copy(void* context, MapValue value) {
    if (value.value_type == MAP_TYPE_STRING) {
        StringInterner* interner = (StringInterner*)context;
        const char* interned_str = string_interner_intern(interner, value.vstring);
        return value_for_string(interned_str);
    }
    return value; // Not a string, do nothing
}

void intern_policy_free(void* context, MapValue value) {
    if (value.value_type == MAP_TYPE_STRING) {
        StringInterner* interner = (StringInterner*)context;
        string_interner_release(interner, value.vstring);
    }
}

// 3. Create the policies struct
ValuePolicies interning_policies = {
    .context = interner,
    .copy = intern_policy_copy,
    .free = intern_policy_free
};

// 4. Create the HashMap, injecting the policies
HashMap* map = map_create(16, &interning_policies);

// To create a map WITHOUT interning, you just pass NULL
HashMap* simple_map = map_create(16, NULL);

*/
