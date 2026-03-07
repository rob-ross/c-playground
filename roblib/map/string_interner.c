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
#include <_string.h>

#include "hashmap_private.h"

// -------------------------------------
// Static/private/internal methods
// ------------------------------------

// Opaque pointer type.
// The full definition is now private to this .c file.
struct InternStringMap {
    HashMap *map;
};

// ------------------------------
// default policy functions
// ------------------------------

//key policies
static MapKey instr_policy_key_add_default(HashMap map[static 1], MapKey key) {
    // default add always make a copy of a string key and we own it
    char *string_copy = strdup(key.kstring);
    return (MapKey){.key_type = MAP_TYPE_STRING, .kstring = string_copy};
}

static void instr_policy_key_free_default(HashMap map[static 1], MapKey key) {
    // default-add always make a copy of a string key and we own it, so we must free it
    free(key.kstring);
}

// value policies
static void instr_policy_value_free_default(HashMap map[static 1], MapValue value) {
    // values are always a long scalar, they don't need to be freed
    // no-op
}

// when this InternStringMap is being used as a string pool for another map, remove operation in enclosing HashMap
// is coupled to this stringpool. Removing a string value in the HashMap means decrementing the count in the
// string pool, not outright freeing it.

[[maybe_unused]] static void instr_policy_value_free_stringpool(InternStringMap ismap[static 1], const MapNode node[static 1]) {
    if ( node->value.value_type == MAP_TYPE_STRING) {
        instr_remove(ismap, node->value.vstring);
    }
}

const MapKeyPolicies   DEFAULT_INSTR_KEY_POLICIES = (MapKeyPolicies){
    .policy_type   = MAP_POLICY_COPY,
    .on_add_key    = instr_policy_key_add_default,
    .on_free_key   = instr_policy_key_free_default,
};

const MapValuePolicies DEFAULT_INSTR_VALUE_POLICIES = (MapValuePolicies){
    .policy_type     = MAP_POLICY_NONE,
    .on_set_value    = nullptr,
    .on_free_value   = nullptr,
};

static void instr_free_key_if(InternStringMap ismap[static 1], const MapNode node[static 1]) {
    if (!ismap || !ismap->map || !node) return;
    HashMap *map = ismap->map;

    if ( map->policies.key_policies.on_free_key ) {
        map->policies.key_policies.on_free_key(map, node->key);
    } else {
        // no key free policy, default is free.
        instr_policy_key_free_default(map, node->key);
    }
}

static void instr_free_value_if(InternStringMap ismap[static 1], const MapNode node[static 1]) {
    if (!ismap || !ismap->map || !node) return;
    HashMap *map = ismap->map;
    if (map->policies.value_policies.on_free_value) {
        map->policies.value_policies.on_free_value(map, node->value);
    } else {
        // no value free policy, default is NO-OP.
        instr_policy_value_free_default(map, node->value);
    }
}

static void instr_destroy_node(InternStringMap ismap[static 1], MapNode *node) {
    if (!ismap) return;
    instr_free_key_if(ismap, node);
    instr_free_value_if(ismap, node);
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
    ismap->map = map_create(num_buckets);
    if (!ismap->map) {
        free(ismap); // Clean up the wrapper if map creation fails
        return nullptr;
    }

    ismap->map->policies.key_policies   = DEFAULT_INSTR_KEY_POLICIES;
    ismap->map->policies.value_policies = DEFAULT_INSTR_VALUE_POLICIES;

    return ismap;
}

void instr_destroy(InternStringMap ismap[static 1]) {
    if (!ismap->map)  return;
    
    HashMap *map = ismap->map;
    // todo in instr_create, we just allocate the InternStringMap wrapper, then delegate to map_create().
    // see if we can refactor this instr_destroy() method to work similarly. The only difference is that if the
    // ismap is being used as a string pool for a parent HashMap, there will be coordination between destroying in the
    // parent and in the child ismap. But this *should* be able to be handled via the policies installed on the
    // parent HashMap.
    instr_clear(ismap); // this frees all allocated MapNodes, keys, and values
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
            instr_destroy_node(ismap, temp);
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
const char* instr_ref(InternStringMap ismap[static 1], char string[static 1]) {
    if (!ismap->map) return nullptr;

    const MapKey key = (MapKey){.kstring = string, .key_type = MAP_TYPE_STRING};
    MapNode * node = map_node_for(ismap->map, key);
    if (!node) {
        // first time string is encountered
        instr_put(ismap, string, 1);
        node = map_node_for(ismap->map, key);
    } else {
        // string in map, increment ref count
        node->value.vlong++;
    }

    if ( !node ) {
        return nullptr;
    }

    return node->key.kstring;
}

//reduce key refcount by 1. When refcount == 0, removes the key from the map
void instr_unref(InternStringMap ismap[static 1], const char *strkey) {
    if (!ismap->map) return;

    const MapKey key = key_for_string(strkey);
    MapNode * node = map_node_for(ismap->map, key);

    if (!node) return ; // key not in map

    if (node->value.vlong - 1 == 0 ) {
        //todo how do we handle negative numbers for references? if we do <= here and user is using negative values,
        //the string will be immediately freed. I.e., if count == -12 and we unref(), does it go to -13 or do we
        // remove it?
        // for now, only +1 -> 0 results in a remove.
        instr_remove(ismap, strkey);
    } else {
        node->value.vlong--; //decrement ref count
    }
}



bool instr_is_empty(InternStringMap ismap[static 1]) {
    return map_is_empty(ismap->map);
}

void instr_put(InternStringMap ismap[static 1], const char* strkey, long value) {
    if ( !ismap->map ) return;

    HashMap *map = ismap->map;
    (map_put)(map, key_for_string(strkey), value_for_long(value));
}


void (instr_remove)(InternStringMap ismap[static 1], const char* strkey) {
    if (!ismap->map) return;

    HashMap *map = ismap->map;

    (map_remove)(map, key_for_string(strkey));
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