// string_interner.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/03 18:22:02 PST

//
// Created by Rob Ross on 3/3/26.
//

#include "string_interner.h"

#include <stdio.h>
#include <stdlib.h>

#include "hashmap_private.h"

// -------------------------------------
// Static/private/internal methods
// ------------------------------------

// Opaque pointer type.
// The full definition is now private to the .c file.
struct InternStringMap {
    HashMap *map;
};


static void instr_destroy_node(MapNode *node) {
    // this is safe, since we always make a copy of a string key and we own it
    // values are always a long scalar, they don't need to be freed
    free(node->key.kstring);
    free(node);
}


// ---------------------------
// Public API methods
// ---------------------------

InternStringMap * instr_create(const size_t num_buckets) {
    // 1. Allocate the wrapper struct.
    InternStringMap *ismap = malloc(sizeof(InternStringMap));
    if (!ismap) {
        return nullptr;
    }
    
    // 2. Create the actual HashMap. Pass nullptr for its own interner
    //    to prevent infinite recursion.
    ismap->map = map_create(num_buckets, nullptr);
    if (!ismap->map) {
        free(ismap); // Clean up the wrapper if map creation fails
        return nullptr;
    }

    return ismap;
}

void instr_destroy(InternStringMap ismap[static 1]) {
    if (!ismap->map)  return;
    
    HashMap *map = ismap->map;
    instr_clear(ismap);
    // all string keys and MapNodes have been freed.
    free(map->buckets);
    map->buckets = nullptr;
    free(map);
    ismap->map = nullptr;
    free(ismap);
}


// deletes all entries and frees them, but does not reduce bucket size or free allocated bucket memory.
void instr_clear(InternStringMap ismap[static 1]) {
    if (!ismap->map) {
        return;
    }
    HashMap *map = ismap->map;
    const size_t num_buckets = map->num_buckets;
    for (size_t i = 0; i < num_buckets; i++) {
        MapNode *current = map->buckets[i];
        while (current) {
            MapNode *temp = current;
            current = current->next;
            instr_destroy_node(temp);
        }
        map->buckets[i] =  nullptr;
    }
    map->load = 0;
    map->size = 0;
}

bool instr_contains_key(InternStringMap ismap[static 1], const char* strkey) {
    if (!ismap->map) return false;
    return (map_contains_key)(ismap->map, key_for_string(strkey));
}



// we are re-implementing map_put so we don't have to do to a get then put value+1 when string is already present
long instr_get_count(const InternStringMap ismap[static 1], const char *key) {
    if (!ismap->map) {
        return 0;
    }
    const MapValue mv = (map_get)(ismap->map, key_for_string(key));
    return mv.vlong;
}

// Ensures the argument string exists in the string_pool.
// Returns a pointer to a const char* that is equal to the `string` argument. Each invocation
// with the same `string` characters will return the same pointer value.
const char* instr_intern(InternStringMap ismap[static 1], char string[static 1]) {
    const MapKey key = (MapKey){.kstring = string, .key_type = MAP_TYPE_STRING};


    MapNode * node = map_node_for(ismap->map, key);
    if (!node) {
        // first time string is encountered
        instr_put(ismap, string);
        node = map_node_for(ismap->map, key);
    } else {
        node->value.vlong++;
    }

    if ( !node ) {
        return nullptr;
    }

    return node->key.kstring;
}


bool instr_is_empty(InternStringMap ismap[static 1]) {
    return map_is_empty(ismap->map);
}

void instr_put(InternStringMap ismap[static 1], const char* strkey) {
    if ( !ismap->map ) return;

    HashMap *map = ismap->map;

    if (map->size >= map->fill_capacity) {
        map_ensure_capacity(map);
    }

    const MapKey key = key_for_string(strkey);

    const size_t hashcode = map_hash_function(key);
    const size_t bucket_index = map_calc_bucket_index(hashcode, map->num_buckets);
    MapNode *current = map->buckets[bucket_index];

    // Check if key already exists and update value
    while (current != nullptr) {
        if ( map_keys_are_equal(key, current->key) ) {
            long value = current->value.vlong + 1;  //increment ref count
            map_set_value(map, current, value_for_long(value));
            return;
        }
        current = current->next;
    }
    // Key not found, insert new node at the beginning of the list
    MapNode *new_node = map_create_node(hashcode, key);
    if (new_node == nullptr) {
        // Handle allocation failure (in a real app, maybe return status)
        return;
    }
    new_node->next =  map->buckets[bucket_index];  // inserts this MapNode at the head of the bucket
    map_set_value(map, new_node, value_for_long(1l));
    map->buckets[bucket_index] = new_node;
    map->size++;
    map_recalc_load(map);
}


//reduce key refcount by 1. When refcount == 0, removes the key from the map
void (instr_remove)(InternStringMap ismap[static 1], const char* strkey) {
    if (!ismap->map) return;

    HashMap *map = ismap->map;

    const MapKey key = key_for_string(strkey);
    const size_t hashcode = map_hash_function(key);
    const size_t index = map_calc_bucket_index(hashcode, map->num_buckets);


    MapNode *current = map->buckets[index];
    MapNode *prev = nullptr;

    while (current) {
        if ( map_keys_are_equal(key, current->key ) ){
            if (current->value.vlong - 1 <= 0) {
                //refcount is now zero, we can free this string and the node
                if (!prev) {
                    map->buckets[index] = current->next;
                } else {
                    prev->next = current->next;
                }
                instr_destroy_node(current);
                map->size--;
                map_recalc_load(map);
            } else {
                current->value.vlong--; // decrement the refcount
            }
            return;
        }
        prev = current;
        current = current->next;

    }
}

size_t instr_size(InternStringMap ismap[static 1]) {
    return map_size(ismap->map);
}


//// ---------------------------------------------
////  repr methods
//// ---------------------------------------------

void instr_repr_InternStringMap(InternStringMap ismap[static 1], const bool verbose) {
    if (!ismap) {
        printf("(InternStringMap)nullptr");
        return;
    }
    map_repr_HashMap(ismap->map, verbose, "InternStringMap");
}