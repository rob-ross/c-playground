// hashmap.c
//
// Copyright (c) Rob Ross 2026.
//
//


// DO NOT INCLUDE string_counter.h. There is a cyclic dependency between that header file and hashmap.h.
// the declarations needed by both string_counter.c and hashmap.c are in hashmap_private.h.

#include "hashmap.h"
#include "hashmap_private.h"
#include "../memory/memory_pool.h"


#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> // Required for memcpy




static constexpr size_t MIN_CAP  = 16;
static constexpr size_t MAX_POW2 = (SIZE_MAX >> 1) + 1;

const MapKey   NULL_MAP_KEY   = (MapKey){  .kvoid_ptr = nullptr, .key_type   = MAP_TYPE_NULL};
const MapValue NULL_MAP_VALUE = (MapValue){ .vvoid_ptr = nullptr, .value_type = MAP_TYPE_NULL};
const MapNode  NULL_MAP_NODE  = (MapNode){ .key = NULL_MAP_KEY, .value = NULL_MAP_VALUE, .hash = 0, .next = nullptr};



// -----------------------------------------------------------------
//      string_counter.h API methods
//
// We can't include string_counter.h due to circular dependencies
// -----------------------------------------------------------------
StringCounter * sct_create(size_t num_buckets);
extern const MapValuePolicy MAP_STRING_POOL_VALUE_POLICIES;



// -----------------------------------------------------------------
//      Forward References
//
// for functions defined in this file
// -----------------------------------------------------------------
static bool map_equals_double(double d1, double d2);
static void map_free_key_if(HashMap map[static 1], const MapNode node[static 1]);
static void map_free_value_if(HashMap map[static 1], const MapNode *node);
static size_t map_hash_mix64(size_t x);
static MapValue map_policy_value_set_default(HashMap map[static 1], MapValue value);
static void map_policy_value_free_default(HashMap map[static 1], MapValue value) ;




//// ------------------------------------------------------------
////
////    Static/private/internal methods
////
//// ------------------------------------------------------------

static bool map_equals_double(double d1, double d2) {
    // Optional policy: NaNs are not equal to anything (including NaN)
    if (isnan(d1) || isnan(d2)) return false;

    // Make -0.0 and +0.0 compare equal (matches common hashing data_policies)
    if (d1 == 0.0) d1 = 0.0;
    if (d2 == 0.0) d2 = 0.0;

    uint64_t ua, ub;
    memcpy(&ua, &d1, sizeof ua);
    memcpy(&ub, &d2, sizeof ub);
    return ua == ub;
}

static bool map_equals_MapValue(const MapValue v1, const MapValue v2) {

    if (v1.value_type != v2.value_type) return false;

    switch (v1.value_type) {
        case MAP_TYPE_NONE:
            return false;
        case MAP_TYPE_LONG:
            return v1.vlong == v2.vlong;
        case MAP_TYPE_DOUBLE:
            return map_equals_double(v1.vdouble, v2.vdouble);
        case MAP_TYPE_STRING:
            return strcmp(v1.vstring, v2.vstring) == 0;
        case MAP_TYPE_VOID_PTR:
            // this requires the caller to have defined an equal function for this blob.
            // todo implement
            return v1.vvoid_ptr == v2.vvoid_ptr;
        case MAP_TYPE_NULL:
            return true; // both value_types are null
        default:
            return false;
    }
}

static void map_free_key_if(HashMap map[static 1], const MapNode node[static 1]) {
    if (!map || !node) return;
    if (map->data_policies.key_policy.on_free_key) {
        map->data_policies.key_policy.on_free_key(map, node->key);
    } else {
        // no key free policy, use default
        map_policy_key_free_default(map, node->key);
    }
}

static void map_free_value_if(HashMap map[static 1], const MapNode node[static 1]) {
    if (!map || !node) return;
    if (map->data_policies.value_policy.on_free_value) {
        map->data_policies.value_policy.on_free_value(map, node->value);
    } else {
        map_policy_value_free_default(map, node->value);
    }
}
// hash mixer
static size_t map_hash_mix64( size_t x ) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

//djb2 hash algorithm, O(N) (actually Theta(N))
static size_t map_hash_string(const char *str) {
    unsigned long hash = 5381; // A "magic" prime number
    int c;

    while (  (c = (unsigned char)(*str++) ) ) {
        // hash = (hash * 33) + c
        // This is a fast way to write it using bit shifts:
        hash = ((hash << 5) + hash) + c;
    }

    return (size_t)hash;
}

static size_t map_next_power_of_two(size_t n) {
    // Clamp lower bound
    if (n < MIN_CAP) return MIN_CAP;

    // Clamp upper bound
    if (n >= MAX_POW2) return MAX_POW2;

    // Round up to next power of two
    // -----------------------------
    // Subtract 1 so exact powers of two don’t round up.
    // Fill all bits below the highest 1 with 1s.
    // Add 1.
    n--;  // clears the lowest set bit and turns all lower bits into 1.
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
#if SIZE_MAX > 0xffffffff
    n |= n >> 32;
#endif
    return n + 1;
}

//// ------------------------------------------------------------
////
////    Default memory policy functions
////
//// ------------------------------------------------------------

void * map_mempolicy_default_malloc( void* context, size_t num_bytes ) {
    return calloc(1, num_bytes);
}

void map_mempolicy_default_free( void* context, void * bytes ) {
    free( bytes) ;
}

void * map_mempolicy_default_allocator_alloc( void* context, size_t num_bytes ) {
    MemoryPool *pool = context;
    return pool_alloc(pool, num_bytes);
}

void  map_mempolicy_default_allocator_free( void * context, void * bytes ) {
    MemoryPool *pool = context;
    pool_free(pool, bytes);
}

const MemPolicy MAP_DEFAULT_MALLOC_POLICY = (MemPolicy){
    .context = nullptr,
    .policy_type = MEM_POLICY_MALLOC_OWN,
    .alloc = map_mempolicy_default_malloc,
    .free = map_mempolicy_default_free,
};

const MemPolicy MAP_DEFAULT_ALLOCATOR_POLICY = (MemPolicy){
    .context = nullptr,  // context gets filled in by actual memory_pool
    .policy_type = MEM_POLICY_ALLOCATOR_OWN,
    .alloc = map_mempolicy_default_allocator_alloc,
    .free = map_mempolicy_default_allocator_free
};



//// ------------------------------------------------------------
////
////    Default data policy functions
////
//// ------------------------------------------------------------

// ------------------------------
// Key data_policies
// ------------------------------

MapKey map_policy_key_add_default(HashMap map[static 1], MapKey key) {
    // default add always make a copy of a string key and we own it
    if (!map) return NULL_MAP_KEY;
    MapPolicyType keypolicy = map->data_policies.key_policy.policy_type;
    if (key.key_type == MAP_TYPE_STRING &&  ( keypolicy == MAP_POLICY_COPY || keypolicy == MAP_POLICY_NONE )) {
        char *string_copy = map_strdup(map->mem_policy, key.kstring);
        return (MapKey){.key_type = MAP_TYPE_STRING, .kstring = string_copy};
    }
    return key;
}

void map_policy_key_free_default(HashMap map[static 1], MapKey key) {
    if (!map) return;
    MapPolicyType keypolicy = map->data_policies.key_policy.policy_type;
    if (key.key_type == MAP_TYPE_STRING &&
        ( keypolicy == MAP_POLICY_COPY || keypolicy == MAP_POLICY_TAKE || keypolicy == MAP_POLICY_NONE )) {
        map_free_bytes(map->mem_policy, key.kstring);  //we own it.
    }
}

// -----------------------
// Value data_policies
// -----------------------

static MapValue map_policy_value_set_default(HashMap map[static 1], MapValue value) {
    // default add always make a copy of a string key and we own it
    if (!map) return NULL_MAP_VALUE;
    MapPolicyType valuepolicy = map->data_policies.value_policy.policy_type;
    if ( value.value_type == MAP_TYPE_STRING &&
        ( valuepolicy == MAP_POLICY_COPY || valuepolicy == MAP_POLICY_NONE )) {
        char *string_copy = map_strdup(map->mem_policy, value.vstring);
        return (MapValue){.value_type = MAP_TYPE_STRING, .vstring = string_copy};
    }
    if ( value.value_type == MAP_TYPE_VOID_PTR ) {
        // invoke the pointer's add method?
        //todo deal with void* types
    }
    return value;
}

static void map_policy_value_free_default(HashMap map[static 1], MapValue value) {
    if (!map) return;
    MapPolicyType valuepolicy = map->data_policies.value_policy.policy_type;
    if ( value.value_type == MAP_TYPE_STRING &&
        ( valuepolicy == MAP_POLICY_COPY || valuepolicy == MAP_POLICY_TAKE || valuepolicy == MAP_POLICY_NONE )) {
        map_free_bytes(map->mem_policy, value.vstring);  //we own it.
    } else if ( value.value_type == MAP_TYPE_VOID_PTR ) {
        // invoke the pointer's free method
        //todo deal with void* types
    }
}

const MapKeyPolicy   MAP_DEFAULT_KEY_POLICY = (MapKeyPolicy){
    .policy_type   = MAP_POLICY_COPY,
    .on_add_key    = map_policy_key_add_default,
    .on_free_key   = map_policy_key_free_default,
    .on_free_context = nullptr,
};

const MapValuePolicy MAP_DEFAULT_VALUE_POLICY = (MapValuePolicy){
    .policy_type     = MAP_POLICY_COPY,
    .on_set_value    = map_policy_value_set_default,
    .on_free_value   = map_policy_value_free_default,
    .on_free_context = nullptr,
};

const MapDataPolicies MAP_DEFAULT_DATA_POLICIES = (MapDataPolicies){
    .key_policy   = MAP_DEFAULT_KEY_POLICY,
    .value_policy = MAP_DEFAULT_VALUE_POLICY
};

//// ------------------------------
//// End default data policy functions
//// ------------------------------




//// ------------------------------------------------------------
////
////    'Package-private' / 'friend' API methods
////
////    declared in hashmap_private.h
//// ------------------------------------------------------------

void * map_alloc_bytes(const MemPolicy mem_policy, const size_t num_bytes) {
    return mem_policy.alloc(mem_policy.context, num_bytes);
}

void  map_free_bytes(const MemPolicy mem_policy, void * bytes) {
    mem_policy.free(mem_policy.context, bytes);
}

size_t map_calc_bucket_index(const size_t hashcode, const size_t num_buckets) {
    return hashcode & (num_buckets - 1);  // works because num_buckets is a power of 2
}

MapNode * map_create_node(HashMap map[static 1], const size_t hashcode, const MapKey key) {

    MapNode *new_node = (MapNode *)map_alloc_bytes(map->mem_policy, sizeof(MapNode));
    if (new_node == nullptr) {
        // Handle allocation failure (in a real app, maybe return status)
        return nullptr;
    }

    MapNode *temp_node;
    switch (key.key_type) {
        case MAP_TYPE_LONG:
            temp_node = &(MapNode){ .hash = hashcode, .key.klong = key.klong, .key.key_type = MAP_TYPE_LONG };
            break;
        case MAP_TYPE_DOUBLE:
            temp_node = &(MapNode){ .hash = hashcode, .key.kdouble = key.kdouble, .key.key_type = MAP_TYPE_DOUBLE };
            break;
        case MAP_TYPE_STRING: {
            MapKey copy;
            if ( map->data_policies.key_policy.on_add_key ) {
                copy = map->data_policies.key_policy.on_add_key(map, key);
            } else {
                copy = map_policy_key_add_default(map, key);
            }
            temp_node = &(MapNode){ .hash = hashcode, .key.kstring = copy.kstring, .key.key_type = MAP_TYPE_STRING };
            break;
        }
        case MAP_TYPE_VOID_PTR:
            temp_node = &(MapNode){ .hash = hashcode, .key.kvoid_ptr = key.kvoid_ptr, .key.key_type = MAP_TYPE_VOID_PTR };
            break;
        case MAP_TYPE_NONE:
        default:
            temp_node = &(MapNode){};
            break;
    }

    memcpy(new_node, temp_node, sizeof(MapNode));

    return new_node;
}

void map_destroy_node(HashMap map[static 1], MapNode node[static 1]) {
    map_free_key_if(map, node);
    map_free_value_if(map, node);
    map_free_bytes(map->mem_policy, node);
}

void map_ensure_capacity(HashMap map[static 1]) {
    const size_t new_num_buckets = map->num_buckets * 2;
    if (new_num_buckets > MAX_POW2) {
        return; // Can't grow anymore
    }

    MapNode **new_buckets = (MapNode **)map_alloc_bytes(map->mem_policy, new_num_buckets * sizeof(MapNode *));
    if (!new_buckets) {
        return; // Allocation failed, keep old map
    }

    // Rehash all existing nodes
    for (size_t i = 0; i < map->num_buckets; i++) {
        MapNode *current = map->buckets[i];
        while (current != nullptr) {
            MapNode *next = current->next; // Save next pointer

            // Calculate new index
            size_t new_index = map_calc_bucket_index(current->hash, new_num_buckets);

            // Insert into new bucket (at head)
            current->next = new_buckets[new_index];
            new_buckets[new_index] = current;

            current = next;
        }
    }

    map_free_bytes(map->mem_policy, map->buckets);
    map->buckets = new_buckets;
    map->num_buckets = new_num_buckets;
    map->fill_capacity = (size_t)(new_num_buckets * (long double)map->fill_factor);
    map_recalc_load(map);
}

size_t map_hash_function(const MapKey key) {
    size_t raw_hash;
    switch (key.key_type) {
        case MAP_TYPE_LONG: {
            // For integer keys, especially sequential ones, we multiply by a large
            // prime to spread the bits out across the 64-bit range. This ensures that
            // the mixer step works effectively. 0x9e3779b97f4a7c15 is a
            // common choice related to the golden ratio.
            raw_hash =  key.klong * 0x9e3779b97f4a7c15ULL;
            break;
        }
        case MAP_TYPE_DOUBLE: {
            double d = key.kdouble;
            // Normalize -0.0 to 0.0 so they hash to the same bucket
            if (d == 0.0) d = 0.0;
            size_t hash = 0;
            if (sizeof(size_t) >= sizeof(double)) {
                memcpy(&hash, &d, sizeof(double));
            } else {
                // ReSharper disable once CppDFAUnreachableCode
                unsigned long long bits;
                memcpy(&bits, &d, sizeof(double));
                // ReSharper disable once CppDFAUnreachableCode
                hash = (size_t)(bits ^ (bits >> 32));
            }
            raw_hash = hash;
            break;
        }
        case MAP_TYPE_STRING: {
            raw_hash = map_hash_string((char*)key.kstring);
            break;
        }
        case MAP_TYPE_VOID_PTR: {
            // this requires the caller to have defined a hash function for this blob.
            raw_hash = (unsigned long)key.kvoid_ptr;
            break;
        }
        case MAP_TYPE_NONE:
        default:
            raw_hash = 0;
    }
    return map_hash_mix64(raw_hash);
}

bool map_equals_MapKey(const MapKey k1, const MapKey k2) {

    if (k1.key_type != k2.key_type) return false;

    switch (k1.key_type) {
        case MAP_TYPE_NONE:
            return false;
        case MAP_TYPE_LONG:
            return k1.klong == k2.klong;
        case MAP_TYPE_DOUBLE:
            return map_equals_double(k1.kdouble, k2.kdouble);
        case MAP_TYPE_STRING:
            return strcmp(k1.kstring, k2.kstring) == 0;
        case MAP_TYPE_VOID_PTR:
            // this requires the caller to have defined an equal function for this blob.
            // todo implement
            return k1.kvoid_ptr == k2.kvoid_ptr;
        case MAP_TYPE_NULL:
            return true; // both key_types are null
        default:
            return false;
    }
}

MapNode * map_node_for(const HashMap map[static 1], const MapKey key) {
    if (map == nullptr) return nullptr;
    const size_t index = map_calc_bucket_index(map_hash_function(key), map->num_buckets);

    MapNode *current = map->buckets[index];

    while (current != nullptr) {
        if ( map_equals_MapKey(key, current->key) ) {
            return current;
        }
        current = current->next;
    }
    return nullptr; // Key not found
}

void map_set_value(HashMap map[static 1], MapNode node[static 1], const MapValue value ) {
    if (value.value_type == MAP_TYPE_STRING) {
        if ( map->data_policies.value_policy.on_set_value ) {
            node->value.vstring = map->data_policies.value_policy.on_set_value(map, value).vstring;
        } else {
            node->value.vstring = map_policy_value_set_default(map, value).vstring;
        }
        node->value.value_type = value.value_type;
    } else {
        node->value = value;
    }
}

void map_recalc_load(HashMap *map) {
    map->load =  (double) ((long double)map->size / map->num_buckets);
}

char * map_strdup(MemPolicy mem_policy, char const * string) {
    const size_t str_len = strlen(string)+1;
    char *dupe = map_alloc_bytes(mem_policy, strlen(string)+1);
    memcpy(dupe, string, str_len);
    return dupe;
}

//// ------------------------------------------------------------
////    End 'Package-private' / 'friend' API methods
//// ------------------------------------------------------------





//// ------------------------------------------------------------
////
////    Public API methods
////
//// ------------------------------------------------------------


// returns nullptr on failure. if num_buckets == 0, uses 16 as initial bucket size.
// num_buckets is clamped to smallest power of two that is greater than num_buckets.
// number of buckets doubles when fill capacity is reached (75% full by default).
// 16 buckets provides adequate sizing for 12 items before growing HashMap capacity
HashMap * (map_create)(size_t num_buckets, MapDataPolicies data_policies, MemPolicy mem_policy) {


    if (!num_buckets) {
        num_buckets = MIN_CAP;
    } else if ( num_buckets > (SIZE_MAX >> 1) + 1) {
        num_buckets = (SIZE_MAX >> 1) + 1;
    } else {
        num_buckets = map_next_power_of_two(num_buckets);
    }

    //todo move this outside the create function. Either the caller provides a pool or we have a helper method that
    // creates one for the caller to be passed in as a mem_policy.
    // size_t initial_pool_size = sizeof(HashMap) +  num_buckets * sizeof(MapNode *) + num_buckets * sizeof(MapNode)*2;
    // MemoryPool *pool = pool_create(initial_pool_size * 3);
    // if (!pool) {
    //     return nullptr;
    // }
    //
    // MemPolicy mem_policy;
    // // mem_policy = MAP_DEFAULT_MALLOC_POLICY;
    // mem_policy = MAP_DEFAULT_ALLOCATOR_POLICY;
    // mem_policy.context = pool;

    HashMap *map = (HashMap *)map_alloc_bytes(mem_policy, sizeof(HashMap));

    if (map == nullptr) {
        return nullptr;
    }


    MapNode **buckets = (MapNode **)map_alloc_bytes(mem_policy, num_buckets * sizeof(MapNode *));
    if (!buckets) {
        map_free_bytes(mem_policy, map);
        return nullptr;
    }

    HashMap prototype = (HashMap){
        .buckets = buckets,
        .size = 0,
        .fill_capacity = (size_t)( num_buckets * (long double)DEFAULT_FILL_FACTOR ),
        .load = 0,
        .num_buckets = num_buckets,
        .fill_factor = DEFAULT_FILL_FACTOR,
        .data_policies= data_policies,
        .mem_policy  = mem_policy,
        .flags = 0 };

    memcpy(map, &prototype, sizeof(HashMap));
    // printf("returnin from map_create: "); map_repr_HashMap(map, false, ""); printf("\n");

    return map;
}


//
// HashMap *(map_create)(size_t num_buckets) {
//
//     //todo initial pool size depends on num_buckets, from which we get fill_capacity, which we multiply by sizeof(MapNode),
//     // and add sizeof(HashMap)
//     // MemoryPool *pool = pool_create(DEFAULT_PAGE_SIZE);
//
//     if (!num_buckets) {
//         num_buckets = MIN_CAP;
//     } else if ( num_buckets > (SIZE_MAX >> 1) + 1) {
//        num_buckets = (SIZE_MAX >> 1) + 1;
//     } else {
//         num_buckets = map_next_power_of_two(num_buckets);
//     }
//
//     size_t initial_pool_size = sizeof(HashMap) +  num_buckets * sizeof(MapNode *) + num_buckets * sizeof(MapNode)*2;
//     MemoryPool *pool = pool_create(initial_pool_size * 3);
//     if (!pool) {
//         return nullptr;
//     }
//
//     MemPolicy mem_policy;
//     // mem_policy = MAP_DEFAULT_MALLOC_POLICY;
//     mem_policy = MAP_DEFAULT_ALLOCATOR_POLICY;
//     mem_policy.context = pool;
//
//     HashMap *map = (HashMap *)map_alloc_bytes(mem_policy, sizeof(HashMap));
//
//     if (map == nullptr) {
//         return nullptr;
//     }
//
//
//     MapNode **buckets = (MapNode **)map_alloc_bytes(mem_policy, num_buckets * sizeof(MapNode *));
//     if (!buckets) {
//         map_free_bytes(mem_policy, map);
//         return nullptr;
//     }
//
//     HashMap prototype = (HashMap){
//         .buckets = buckets,
//         .size = 0,
//         // todo fill_capacity calculation probably needs to use num_buckets and not MIN_CAP here?
//         .fill_capacity = (size_t)( num_buckets * (long double)DEFAULT_FILL_FACTOR ),
//         .load = 0,
//         .num_buckets = num_buckets,
//         .fill_factor = DEFAULT_FILL_FACTOR,
//         .mem_policy  = mem_policy,
//         .flags = 0 };
//
//     prototype.data_policies.key_policy   = MAP_DEFAULT_KEY_POLICY;
//     prototype.data_policies.value_policy = MAP_DEFAULT_VALUE_POLICY;
//
//     memcpy(map, &prototype, sizeof(HashMap));
//     // printf("returnin from map_create: "); map_repr_HashMap(map, false, ""); printf("\n");
//
//     return map;
// }

HashMap *map_create_using_stringpool(size_t num_buckets) {
    HashMap *map = map_create(num_buckets);
    if (!map) return nullptr;

    StringCounter *sct = sct_create(num_buckets);

    if (!sct) {
        map_destroy(map);
        return nullptr;
    }

    map->data_policies.key_policy = MAP_DEFAULT_KEY_POLICY;
    map->data_policies.value_policy = MAP_STRING_POOL_VALUE_POLICIES;

    //add the string pool map as the context for the enclosing HashMap's value policy
    map->data_policies.value_policy.context = sct;

    return map;
}

[[maybe_unused]]
HashMap *map_create_using_allocator(size_t num_buckets) {
    return nullptr;
}

void map_destroy(HashMap map[static 1]) {
    if (!map) return;

    map_clear(map);

    // free any data policy contexts that exist
    if (map->data_policies.key_policy.on_free_context) {
        map->data_policies.key_policy.on_free_context(map->data_policies.key_policy.context);
    }
    map->data_policies.key_policy.context = nullptr;

    if (map->data_policies.value_policy.on_free_context) {
        map->data_policies.value_policy.on_free_context(map->data_policies.value_policy.context);
    }
    map->data_policies.value_policy.context = nullptr;

    map_free_bytes(map->mem_policy, map->buckets);
    map->buckets = nullptr;

    MemPolicy mem_policy = map->mem_policy;

    map_free_bytes(map->mem_policy, map);

    if (mem_policy.context ) {
        if (mem_policy.free_context) {
            if (mem_policy.policy_type == MEM_POLICY_ALLOCATOR_OWN) {
                mem_policy.free_context(mem_policy.context);
            }
        } else {
            if (mem_policy.policy_type == MEM_POLICY_MALLOC_OWN) {
                free(mem_policy.context);
            }
        }
    }
}

// deletes and frees all Nodes but does not reduce bucket size or free allocated bucket memory.
// todo some kind of resize method to realloc to a smaller memory footprint?
void map_clear(HashMap map[static 1]) {
    const size_t num_buckets = map->num_buckets;
    for (size_t i = 0; i < num_buckets; i++) {
        MapNode *current = map->buckets[i];
        while (current != nullptr) {
            MapNode *temp = current;
            current = current->next;
            map_destroy_node(map, temp);
        }
        map->buckets[i] =  nullptr;
    }
    map->load = 0;
    map->size = 0;
}

bool (map_contains_key)(HashMap map[static 1], const MapKey key) {
    MapValue unused;
    return (map_try_get)(map, key, &unused);
}


// initial implementation is O(N)
bool (map_contains_value)(HashMap map[static 1], const MapValue value) {
    const size_t num_buckets = map->num_buckets;
    for ( size_t index = 0; index < num_buckets; ++index ) {
        const MapNode *current = map->buckets[index];
        while (current) {
            if ( map_equals_MapValue(value, current->value) ) {
                return current;
            }
            current = current->next;
        }
    }
    return false;
}


MapValue (map_get)(const HashMap map[static 1], const MapKey key) {
    if (map == nullptr) return NULL_MAP_VALUE;
    const size_t index = map_calc_bucket_index(map_hash_function(key), map->num_buckets);

    MapNode const *current = map->buckets[index];

    while (current != nullptr) {
        if ( map_equals_MapKey(key, current->key) ) {
            return current->value;
        }
        current = current->next;
    }
    return NULL_MAP_VALUE; // Key not found
}

MapValue (map_get_or)(const HashMap map[static 1], const MapKey key, const MapValue fallback) {
    const MapValue value = (map_get)(map, key);
    if (value.value_type == MAP_TYPE_NULL) {
        return fallback;
    }
    return value;
}

bool (map_try_get)(const HashMap map[static 1], const MapKey key, MapValue *out) {
    const MapValue value = (map_get)(map, key);
    if (value.value_type == MAP_TYPE_NULL) {
        *out = NULL_MAP_VALUE;
        return false;
    }
    *out = value;
    return true;
}

bool map_is_empty(const HashMap map[static 1]) {
    return map->size == 0;
}

// if key or value are strings, they are copied so the map can be free them independently of the original arguments.
void (map_put)(HashMap map[static 1], const MapKey key, const MapValue value) {
    if (map == nullptr) return;

    if (map->size >= map->fill_capacity) {
        map_ensure_capacity(map);
    }

    const size_t hashcode = map_hash_function(key);
    const size_t bucket_index = map_calc_bucket_index(hashcode, map->num_buckets);
    MapNode *current = map->buckets[bucket_index];

    // Check if key already exists and update value
    while (current != nullptr) {
        if ( map_equals_MapKey(key, current->key) ) {
            map_free_value_if(map, current);
            map_set_value(map, current, value);
            return;
        }
        current = current->next;
    }
    // Key not found, insert new node at the beginning of the list
    MapNode *new_node = map_create_node(map, hashcode, key);
    if (new_node == nullptr) {
        // Handle allocation failure (in a real app, maybe return status)
        return;
    }
    new_node->next =  map->buckets[bucket_index];  // inserts this MapNode at the head of the bucket
    map_set_value(map, new_node, value);
    map->buckets[bucket_index] = new_node;
    map->size++;
    map_recalc_load(map);

}

void (map_remove)(HashMap map[static 1], const MapKey key) {
    if (!map) return;

    const size_t hashcode = map_hash_function(key);
    const size_t index = map_calc_bucket_index(hashcode, map->num_buckets);

    MapNode *current = map->buckets[index];
    MapNode *prev = nullptr;

    while (current) {
        if ( map_equals_MapKey(key, current->key ) ){
            if (!prev) {
                map->buckets[index] = current->next;
            } else {
                prev->next = current->next;
            }
            map_destroy_node(map, current);
            map->size--;
            map_recalc_load(map);
            return;
        }
        prev = current;
        current = current->next;
    }
}

size_t map_size(const HashMap map[static 1]) {
    return map->size;
}

//// ---------------------------------------------
////  repr methods
//// ---------------------------------------------

void map_repr_HashMap(const HashMap map[static 1], const bool verbose, const char* type_str) {
    if (!type_str || type_str[0] == '\0') type_str = "HashMap";
    if (!map) {
        printf("(%s)nullptr",type_str);
        return;
    }

    // ReSharper disable CppPrintfBadFormat
    // ReSharper disable CppPrintfExtraArg
    printf( "(%s){ .size=%'zu, .fill_capacity=%'zu, .load=%'g, "
            ".num_buckets=%'zu, .fill_factor=%g, .mem_policy_type = %hhd"
            "}", type_str,
            map->size, map->fill_capacity, map->load,
            map->num_buckets, map->fill_factor, map->mem_policy.policy_type);


    // we might want a special Bucket struct to be the head of each bucket, so we can keep statistics on the bucket
    // contents
    // For now we'll do it the hard way and generate stats on the fly by visiting every MapNode in the HashMap
    printf("\n");
    if (verbose) {
        constexpr int max_buckets_displayed = 16;
        size_t total_bucket_items = 0;
        printf("bucket sizes: ");
        for (size_t i=0; i < map->num_buckets; ++i) {
            const MapNode *current = map->buckets[i];
            size_t node_count = 0;
            while (current) {
                node_count++;
                current = current->next;
            }
            total_bucket_items += node_count;
            if ( i < max_buckets_displayed) {
                printf("[%zu]=%'zu, ", i, node_count);
            }
        }
        if ( map->num_buckets > max_buckets_displayed) {
            printf(" ...");
        }
        printf("\n");
        printf("total bucket items: %'zu\n\n", total_bucket_items);


        // for each bucket, display name/value pairs.
        for (size_t i=0; i < max_buckets_displayed && i < map->num_buckets; ++i) {
            const MapNode *current = map->buckets[i];
            printf("bucket[%zu]:",i);
            if (!current) {
                printf(" empty\n");
            } else {
                printf("\n");
            }
            size_t node_count = 0;
            while ( current) {
                node_count++;
                if ( node_count < max_buckets_displayed) {
                    map_repr_Node(current);
                }
                current = current->next;
            }
            printf("---------------------------------------------------------------------------------------------------------------------\n");
            printf("bucket[%zu] node count:%zu\n",i,node_count);
            printf("\n");
        }
    }

}

void map_repr_MapKey(const MapKey map_key, bool verbose) {

    if (map_key.key_type == MAP_TYPE_NONE) {
        printf("(MapKey){ MAP_TYPE_NONE }");
        return;
    }
    if (map_key.key_type == MAP_TYPE_NULL) {
        printf("(MapKey){ MAP_TYPE_NULL }");
        return;
    }
    if (verbose) {
        printf("(MapKey){ ");
    }
    switch (map_key.key_type) {
        case MAP_TYPE_LONG:
            printf(".klong=%5lu", map_key.klong);
            break;
        case MAP_TYPE_DOUBLE:
            printf(".kdouble=%5g", map_key.kdouble);
            break;
        case MAP_TYPE_STRING:
            printf(".kstring='%5s'", map_key.kstring);
            break;
        case MAP_TYPE_VOID_PTR:
            printf(".kvoid_ptr=%14p", map_key.kvoid_ptr);
            break;
        default:
            if (verbose) {
                printf("unknown");
            } else {
                printf("(MapKey){ unknown }");
            }
    }
    if (verbose) {
        printf(" }");
    }
}

void map_repr_MapValue(const MapValue map_value, const bool verbose) {
    if (map_value.value_type == MAP_TYPE_NONE) {
        printf("(MapValue){ MAP_TYPE_NONE }");
        return;
    }
    if (map_value.value_type == MAP_TYPE_NULL) {
        printf("(MapValue){ MAP_TYPE_NULL }");
        return;
    }
    if (verbose) {
        printf("(MapValue){ ");
    }
    switch (map_value.value_type) {
        case MAP_TYPE_LONG:
            printf(".vlong=%5lu", map_value.vlong);
            break;
        case MAP_TYPE_DOUBLE:
            printf(".vdouble=%5g", map_value.vdouble);
            break;
        case MAP_TYPE_STRING:
            printf(".vstring='%5s'", map_value.vstring);
            break;
        case MAP_TYPE_VOID_PTR:
            printf(".vvoid_ptr=%14p", map_value.vvoid_ptr);
            break;
        default:
            if (verbose) {
                printf("unknown");
            } else {
                printf("(MapValue){ unknown }");
            }
    }
    if (verbose) {
        printf(" }");
    }
}

void map_repr_Node(const MapNode node[static 1]) {
    if (!node) {
        printf("(MapNode)nullptr");
        return;
    }
    printf("(MapNode){ ");
    printf(".hash=0x%-16lX", node->hash);
    printf(", .this=%14p, .next=%14p, ", node, node->next);
    map_repr_MapKey(node->key, false);
    printf(", ");
    map_repr_MapValue(node->value, false);
    printf(" }\n");
}





//// -----------------------------------------------------
////    Converters for generic map function arguments
////
////    these convert expressions to a MapKey or MapValue
//// -----------------------------------------------------

MapKey key_for_long(const long k) {
    return (MapKey){.klong = k, .key_type = MAP_TYPE_LONG};
}

MapKey key_for_double(const double k) {
    return (MapKey){.kdouble = k, .key_type = MAP_TYPE_DOUBLE};
}

MapKey key_for_string(const char * k) {
    return (MapKey){.kstring = (char*)k, .key_type = MAP_TYPE_STRING};
}

MapKey key_for_void_ptr(const void * k) {
    return (MapKey){.kvoid_ptr = (void*)k, .key_type = MAP_TYPE_VOID_PTR};
}

MapValue value_for_long(const long v) {
    return (MapValue){.vlong = v, .value_type = MAP_TYPE_LONG};
}

MapValue value_for_double(const double v) {
    return (MapValue){.vdouble = v, .value_type = MAP_TYPE_DOUBLE};
}

MapValue value_for_string(const char * v) {
    return (MapValue){.vstring = (char*)v, .value_type = MAP_TYPE_STRING};
}

MapValue value_for_void_ptr(const void * v) {
    return (MapValue){.vvoid_ptr = (void*)v, .value_type = MAP_TYPE_VOID_PTR};
}
