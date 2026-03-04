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

static void instr_destroy_node(MapNode *node) {
    // this is safe, since we always make a copy of a string key and we own it
    free(node->key.kstring);
    free(node);
}



// ---------------------------
// Public API methods
// ---------------------------

InternStringMap instr_create() {
    HashMap *map = map_create(0);

    return (InternStringMap){.map = map};

}

// deletes all entries and frees them, but does not reduce bucket size or free allocated bucket memory.
void instr_clear(InternStringMap ismap) {
    if (!ismap.map) {
        return;
    }
    HashMap *map = ismap.map;
    for (size_t i = 0; i < map->num_buckets; i++) {
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

bool instr_contains_key(InternStringMap ismap, const char* strkey) {
    if (!ismap.map) return false;
    return (map_contains_key)(ismap.map, key_for_string(strkey));
}

void instr_destroy(InternStringMap ismap) {
    if (!ismap.map) {
        return;
    }
    HashMap *map = ismap.map;

    const size_t num_buckets = map->num_buckets;
    for (size_t i = 0; i < num_buckets; ++i) {
        MapNode *current = map->buckets[i];
        while (current) {
            MapNode *temp = current;
            current = current->next;
            instr_destroy_node(temp);
        }
        map->buckets[i] =  nullptr;
    }
    // all string keys and MapNodes have been freed.
    free(map->buckets);
    map->buckets = nullptr;
    free(map);
    ismap.map = nullptr;
}

// we are re-implementing map_put so we don't have to do to a get then put value+1 when string is already present
MapValue (instr_get_count)(const InternStringMap ismap, const char* key) {
    if (!ismap.map) {
        return NULL_MAP_VALUE;
    }
    return (map_get)(ismap.map, key_for_string(key));
}


bool instr_is_empty(InternStringMap ismap) {
    return map_is_empty(ismap.map);
}

void instr_put(InternStringMap ismap, const char* strkey) {
    if ( !ismap.map ) return;

    HashMap *map = ismap.map;

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
void (instr_remove)(InternStringMap ismap, const char* strkey) {
    if (!ismap.map) return;

    HashMap *map = ismap.map;

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

size_t instr_size(InternStringMap ismap) {
    return map_size(ismap.map);
}