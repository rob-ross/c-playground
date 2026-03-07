// string_counter.h
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/03 15:47:03 PST

// ------------------------------------------------------------------------------------------------------
//      StringCounter
//
// This is a HashMap that has additional functions for treating the
// map as a Python Counter or a Bag/Multiset.
// It is a collection where elements are stored as map keys and their counts are stored as map values.
// Counts are allowed to be any integer value including zero or negative counts.
// map_get() returns the count for a key as a MapValue
// sct.get_count() returns the count for a key as a long
// sct.ref() adds the key if not present, or increments the count if present.
// sct.unref() reduces the count by one. When count == 0, the key is removed from the map and freed.
// ------------------------------------------------------------------------------------------------------

#pragma once
#include "hashmap.h"

// Forward-declare the struct to make it an opaque type.
// Users can only have pointers to it, not instances.
// Full definition in string_counter.c
struct StringCounter;
typedef struct StringCounter StringCounter;


// ---------------------------
// Public API methods
// ---------------------------

StringCounter * sct_create(size_t num_buckets);
void sct_clear(StringCounter *sct);
bool sct_contains_key(StringCounter *sct, const char strkey[static 1]);
void sct_destroy(StringCounter *sct);

long sct_get_count(const StringCounter *sct, const char string[static 1]);
bool sct_is_empty(StringCounter *sct);
void sct_put(StringCounter *sct, const char string[static 1], long value);

// Increases the reference count of the string key.
// If the string key is not in the map, adds it and sets refcount to 1
const char* sct_ref(StringCounter *sct, char string[static 1] ) ;
//Decreases the reference count of the string. When its reference count drops to 0, the object is freed
void sct_unref(StringCounter *sct, const char string[static 1] );

//Removes the mapping for a key from this map if it is present
//Returns the value to which this map previously associated the key, or null if the map contained no mapping for the key.
void sct_remove(StringCounter *sct, const char string[static 1] );
size_t sct_size(StringCounter *sct);

void sct_repr_StringCounter(StringCounter *sct, bool verbose);

