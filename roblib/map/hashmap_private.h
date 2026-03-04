// hashmap_private.h
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/03 19:48:12 PST

//
// Shared private API for HashMap objects
//

#pragma once

size_t map_calc_bucket_index(size_t hashcode, size_t num_buckets);
MapNode * map_create_node(size_t hashcode, MapKey key) ;
void map_ensure_capacity(HashMap *map);
size_t map_hash_function(MapKey key);
// Compare the arguments for equality. Assumes k1 and k2 are of the same type.
bool map_keys_are_equal(MapKey k1, MapKey k2);
void map_set_value(const HashMap *top_map, MapNode *node, MapValue value );
void map_recalc_load(HashMap *map);