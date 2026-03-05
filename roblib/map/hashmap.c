// hashmap.c
//
// Copyright (c) Rob Ross 2026.
//
//

#include "hashmap.h"
#include "hashmap_private.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> // Required for memcpy


static constexpr size_t MIN_CAP  = 16;
static constexpr size_t MAX_POW2 = (SIZE_MAX >> 1) + 1;

const MapKey   NULL_MAP_KEY   = (MapKey){  .kvoid_ptr = nullptr, .key_type   = MAP_TYPE_NULL};
const MapValue NULL_MAP_VALUE = (MapValue){.vvoid_ptr = nullptr, .value_type = MAP_TYPE_NULL};
const MapNode NULL_MAP_NODE = (MapNode){ .key = NULL_MAP_KEY, .value = NULL_MAP_VALUE, .hash = 0, .next = nullptr};


// forward references
static bool map_equals_double(double d1, double d2);
static void map_destroy_node(HashMap map[static 1], MapNode *node);
static void map_free_value_if(const HashMap *map, const MapNode *node);
static size_t map_hash_string(const char *str);
static size_t map_hash_mix64(size_t x);

// string_interner.h API methods used here. We can't include string_interner.h due to circular dependencies
void instr_destroy(InternStringMap *ismap);
const char* instr_intern(InternStringMap *string_pool, char string[static 1]);
void instr_remove(InternStringMap *ismap, const char* strkey);
size_t instr_size(InternStringMap *ismap);

// -------------------------------------
// Static/private/internal methods
// ------------------------------------

static bool map_equals_double(double d1, double d2) {
    // Optional policy: NaNs are not equal to anything (including NaN)
    if (isnan(d1) || isnan(d2)) return false;

    // Make -0.0 and +0.0 compare equal (matches common hashing policies)
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




// -------------------------------------
// 'Package-private/friend' API methods
// -------------------------------------
//  declared in hashmap_private.h
//

size_t map_calc_bucket_index(const size_t hashcode, const size_t num_buckets) {
    return hashcode & (num_buckets - 1);  // works because num_buckets is a power of 2
}

MapNode * map_create_node(const size_t hashcode, const MapKey key) {

    MapNode *new_node = (MapNode *)malloc(sizeof(MapNode));
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
            //we always copy the string used as a key.
            // todo this should be governed by the KeyPolicy, for things like an IdentityHashMap impl
            char *string_copy = strdup(key.kstring);
            temp_node = &(MapNode){ .hash = hashcode, .key.kstring = string_copy, .key.key_type = MAP_TYPE_STRING };
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

static void map_free_value_if(const HashMap map[static 1], const MapNode node[static 1]) {
    if ( node->value.value_type == MAP_TYPE_STRING) {
        if ( map->string_pool) {
            //todo the ValuePolicy should govern this next call:
            instr_remove(map->string_pool, node->value.vstring);
        } else {
            // we're not using a string pool, so we made a copy of the string. We can free it.
            //todo this should be controlled by ValuePolicy
            free( node->value.vstring) ;
        }
    }
    else if ( node->value.value_type == MAP_TYPE_VOID_PTR ) {
        // invoke the pointer's free method
    }
}

static void map_destroy_node(HashMap map[static 1], MapNode node[static 1]) {
    if (node->key.key_type == MAP_TYPE_STRING) {
        // this is safe, since we always make a copy of a string key and we own it
        // todo this should be controlled by KeyPolicy
        free(node->key.kstring);
    }
    map_free_value_if(map, node);
    free(node);
}

void map_ensure_capacity(HashMap map[static 1]) {
    const size_t new_num_buckets = map->num_buckets * 2;
    if (new_num_buckets > MAX_POW2) {
        return; // Can't grow anymore
    }

    MapNode **new_buckets = (MapNode **)calloc(new_num_buckets, sizeof(MapNode *));
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

    free(map->buckets);
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

bool map_keys_are_equal(const MapKey k1, const MapKey k2) {

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
        if ( map_keys_are_equal(key, current->key) ) {
            return current;
        }
        current = current->next;
    }
    return nullptr; // Key not found
}

void map_set_value(const HashMap map[static 1], MapNode node[static 1], const MapValue value ) {
    switch (value.value_type) {
        case MAP_TYPE_LONG:
            node->value.vlong = value.vlong;
            break;
        case MAP_TYPE_DOUBLE:
            node->value.vdouble = value.vdouble;
            break;
        case MAP_TYPE_STRING:
            if (map->string_pool) {
                //get string from intern string pool
                // we need the void* cast because instr_intern returns const char*.
                // todo make value.vstring const char*
                node->value.vstring = (void *) instr_intern(map->string_pool, value.vstring);
            } else {
                //todo ValuePolicy should control this.
                // here, if there is no string_pool, we will copy the string to be safe.
                node->value.vstring = strdup(value.vstring);
            }
            break;
        case MAP_TYPE_VOID_PTR:
            node->value.vvoid_ptr = value.vvoid_ptr;
            break;
        case MAP_TYPE_NONE:
        case MAP_TYPE_NULL:
        default:
            node->value.vvoid_ptr = nullptr;
            break;
    }
    node->value.value_type = value.value_type;
}

void map_recalc_load(HashMap *map) {
    map->load =  (double) ((long double)map->size / map->num_buckets);
}




// ---------------------------
// Public API methods
// ---------------------------

// returns nullptr on failure. if num_buckets == 0, uses 16 as initial bucket size.
// num_buckets is clamped to smalles power of two > num_buckets.
// buckets double when fill capacity is reached (75% full). 16 buckets provides adequate sizing for 12 items before
// doubling.
HashMap *map_create(size_t num_buckets, InternStringMap * string_pool) {

    HashMap *map = (HashMap *)malloc(sizeof(HashMap));
    if (map == nullptr) {
        return nullptr;
    }
    if (!num_buckets) {
        num_buckets = MIN_CAP;
    } else if ( num_buckets > (SIZE_MAX >> 1) + 1) {
       num_buckets = (SIZE_MAX >> 1) + 1;
    } else {
        num_buckets = map_next_power_of_two(num_buckets);
    }

    MapNode **buckets = (MapNode **)calloc(num_buckets, sizeof(MapNode *));
    if (!buckets) {
        free(map);
        return nullptr;
    }

    HashMap prototype = (HashMap){
        .buckets = buckets,
        .size = 0,
        .fill_capacity = (size_t)( MIN_CAP * (long double)DEFAULT_FILL_FACTOR ),
        .load = 0,
        .num_buckets = num_buckets,
        .fill_factor = DEFAULT_FILL_FACTOR,
        .string_pool = string_pool,
        .flags = 0 };

    memcpy(map, &prototype, sizeof(HashMap));

    return map;
}

void map_destroy(HashMap map[static 1]) {
    if (!map) return;
    map_clear(map);
    free(map->buckets);
    map->buckets = nullptr;
    //todo this shouldn't be a dependency here, but part of the ValuePolicy?
    if (map->string_pool) {
        instr_destroy(map->string_pool);
        map->string_pool = nullptr;
    }

    free(map);
}

// deletes and frees all Nodes but does not reduce bucket size or free allocated bucket memory.
// todo some kind of resize method to realloc to a smaller memory footprint?
void map_clear(HashMap map[static 1]) {
    for (size_t i = 0; i < map->num_buckets; i++) {
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
        if ( map_keys_are_equal(key, current->key) ) {
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
        if ( map_keys_are_equal(key, current->key) ) {
            map_free_value_if(map, current);
            map_set_value(map, current, value);
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
        if ( map_keys_are_equal(key, current->key ) ){
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
            ".num_buckets=%'zu, .fill_factor=%g, "
            ".string_pool=%p }", type_str,
            map->size, map->fill_capacity, map->load,
            map->num_buckets, map->fill_factor, map->string_pool);


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

size_t map_size(const HashMap map[static 1]) {
    return map->size;
}



//// ---------------------------------------------------
//// Converters for generic map function arguments
////  these convert expressions to a MapKey or MapValue
//// ---------------------------------------------------

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
