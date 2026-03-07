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
struct StringCounter;
typedef struct StringCounter StringCounter;


// ---------------------------
// Public API methods
// ---------------------------

StringCounter * sct_create(size_t num_buckets);
void sct_clear(StringCounter *ismap);
bool sct_contains_key(StringCounter *ismap, const char* strkey);
void sct_destroy(StringCounter *ismap);

long sct_get_count(const StringCounter *ismap, const char *key);
bool sct_is_empty(StringCounter *ismap);
void sct_put(StringCounter *ismap, const char* strkey, long value);

// Increases the reference count of the string key.
// If the string key is not in the map, adds it and sets refcount to 1
const char* sct_ref(StringCounter *ismap, char string[static 1]) ;
//Decreases the reference count of the string. When its reference count drops to 0, the object is finalized (i.e. its memory is freed).
void sct_unref(StringCounter *ismap, const char *strkey);

void sct_remove(StringCounter *ismap, const char* strkey);
size_t sct_size(StringCounter *ismap);

void sct_repr_InternStringMap(StringCounter *ismap, bool verbose);

/*
 *
*    typedef struct MapValuePolicies {
        // A context pointer to be passed to the policy functions.
        void* context;

        // Called when a value is added. Returns the value to be stored.
        // Can be used for copying, interning, or reference counting.
        MapValue (*copy)(void* context, MapValue value);

        // Called when a value is removed or the map is freed.
        void (*free)(void* context, MapValue value);
    } MapValuePolicies;

if (map->value_policies.copy) {
            value_to_store = map->value_policies.copy(map->value_policies.context, new_value);
        }

if (map->value_policies.free) {
            map->value_policies.free(map->value_policies.context, old_value);
        }


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
MapValuePolicies interning_policies = {
    .context = interner,
    .copy = intern_policy_copy,
    .free = intern_policy_free
};

// 4. Create the HashMap, injecting the policies
HashMap* map = map_create(16, &interning_policies);

// To create a map WITHOUT interning, you just pass NULL
HashMap* simple_map = map_create(16, NULL);

*/
