// set.h
//
// Copyright (c) Rob Ross 2026.
//
//

//
// Created by Rob Ross on 3/3/26.
//

#pragma once


#include "../map/hashmap.h" // Your existing hash map

typedef struct Set {
    HashMap *map;
} Set;

typedef struct SetItem {
    MapKey item;
} SetItem;

static const MapValue DUMMY_VALUE = (MapValue){ .vlong = 0, .value_type = MAP_TYPE_LONG};

// ----------------------------------
// API functions
// ----------------------------------

// Creates a new, empty set.
Set set_create(size_t num_buckets);

// Frees all memory used by the set.
void set_free(Set *set);

// Adds an item to the set. If the item is already present, the set is unchanged.
void set_add(const Set *set, SetItem item);

// Returns true if the set contains the specified item, false otherwise.
bool set_contains(const Set *set, SetItem item);

// Returns true if the set contains no items.
bool set_is_empty(const Set *set);

// Removes the item from the set if it is present.
void set_remove(const Set *set, SetItem item);

// Returns the number of items in the set.
size_t set_size(const Set *set);
