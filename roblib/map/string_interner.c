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
#include "hashmap_private.h"

InternStringMap intstr_create() {
    HashMap *map = map_create(0);

    return (InternStringMap){.map = map};

}


// we are re-implementing map_put so we don't have to do to a get then put value+1 when string is already present
void intstr_put(InternStringMap ismap[static 1], const char* strkey) {
    if ( !ismap || !ismap->map ) return;

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

MapValue (intstr_get)(const InternStringMap map[static 1], const char* key) {

}