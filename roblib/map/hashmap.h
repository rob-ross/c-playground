// hashmap.h
//
// Copyright (c) Rob Ross 2026.
//
//

#pragma once

#include <stddef.h> // For size_t
#include <stdint.h> // For uint8_t


// we can support arbitrary objects via a void* key, but we'll save that for later
// for values we should support all the the scalar types, which we can do via int/long and double.
// for values we will also support char* specifically for strings, and void* for arbitrary data.

// in phase 3 we'll optimize memory by allocating in chunks so we can keep our buckets
// and linked lists in the same memory chunk.



typedef enum MapTypeEnum: unsigned char {
    MAP_TYPE_NONE,
    MAP_TYPE_LONG,
    MAP_TYPE_DOUBLE,
    MAP_TYPE_STRING,
    MAP_TYPE_VOID_PTR,
    MAP_TYPE_NULL
} MapTypeEnum;

typedef struct BasicBlob {
    size_t size;
    uint8_t *data;
} BasicBlob;

// if we really intend to support *any* data buffer type as a key in the map, we need to be provided:
// 1. a free function pointer, if we need to free it.
// 2. ownership flags? Map owns it or caller owns it? Other values.... for char* maybe "is literal" as a guard to not free.
// 3. do we copy the blob or not? If we're going to own it without transfer of ownership, we'll have to make a copy.
//      if the caller is transfering ownership, we won't need to copy it.
// 4. hash function and is_equal method. Perhaps a compare as well.
// for now we won't support void* as a map key. We copy all strings and free the copies when we're done with them.
// 16 bytes
typedef struct MapKey {
    union {
        long klong;
        double kdouble;
        char *kstring;
        void *kvoid_ptr;
    };
    MapTypeEnum  key_type;
} MapKey;

// 16 bytes
typedef struct MapValue {
    union {
        long   vlong;
        double vdouble;
        char  *vstring;
        void  *vvoid_ptr;
    };
    MapTypeEnum  value_type;
} MapValue;


// 48
typedef struct Node {
    const MapKey key;
    MapValue     value;
    const size_t hash;
    struct Node  *next;
} Node;


struct HashMap;
// Define the HashMap structure
// 64 bytes
typedef struct HashMap {
    Node **buckets;               // each bucket is a linked list
    size_t size;                  // Number of key-value pairs currently in the map
    size_t fill_capacity;         // holds the max ideal size for the current number of buckets. Increases when buckets increase
    double load;                  // current load = size / num_buckets
    size_t num_buckets;           // must always be a power of 2
    double fill_factor;           // desired load

    struct HashMap * intern_strings;
    void (*free_func)(void *); // Function pointer to free allocated values
    uint64_t flags; // future use
} HashMap;

constexpr MapKey   NULL_MAP_KEY   = (MapKey){  .kvoid_ptr = nullptr, .key_type   = MAP_TYPE_NULL};
constexpr MapValue NULL_MAP_VALUE = (MapValue){.vvoid_ptr = nullptr, .value_type = MAP_TYPE_NULL};

static constexpr Node NULL_NODE = (Node){ .key = NULL_MAP_KEY, .value = NULL_MAP_VALUE, .hash = 0, .next = nullptr};
static constexpr double DEFAULT_FILL_FACTOR = 0.75;

// ---------------------------
// API methods
// ---------------------------

HashMap *map_create(size_t num_buckets, void (*free_value_func)(void *));
//Removes all of the mappings from this map. Keeps existing buckets. After call size == 0.
void map_clear(HashMap map[static 1]);
//  Returns true if this map contains a mapping for the specified key.
// if you intend to use the key's value immediately if it exists, consider using map_try_get instead, for efficiency.
bool map_contains_key(HashMap map[static 1], MapKey key) ;
//  Returns true if this map contains a mapping for the specified key.
bool map_contains_value(HashMap map[static 1], MapValue value);

void map_free(HashMap map[static 1]);

// Returns the value for the given key.
// If no value found for the key, MapValue.value_type === MAP_TYPE_NULL and MapValue.vvoidptr == nullptr
MapValue (map_get)(const HashMap map[static 1], const MapKey key);
// Returns the value to which the specified key is mapped, or fallback if this map contains no mapping for the key.
MapValue (map_get_or)(const HashMap map[static 1], const MapKey key, const MapValue fallback) ;
// if key exists, copies the value into out and returns true. If key does not exist, writes
bool (map_try_get)(const HashMap map[static 1], MapKey key, MapValue *out);

// Returns true if this map contains no key-value mappings.
bool map_is_empty(const HashMap map[static 1]);


//associates value with key in the map. If key did not previously exist in the map, this
// function adds it. If the key already exists, the value is replaced with the argument value.
// If the key previously existed, the old value is returned. If the key is being added, returns
//NULL_MAP_VALUE.
void map_put(HashMap map[static 1], MapKey key, MapValue value) ;

//Removes the mapping for a key from this map if it is present
// Returns the value to which this map previously associated the key, or null if the map contained no mapping for the key.
void (map_remove)(HashMap map[static 1], MapKey key);

//// ---------------------------------------------
////  repr methods
//// ---------------------------------------------
void map_repr_HashMap(const HashMap map[static 1], bool verbose);
void map_repr_MapKey(MapKey map_key, bool verbose);
void map_repr_MapValue(MapValue map_value, bool verbose);
void map_repr_Node(const Node node[static 1]);

// Returns the number of key-value mappings in this map.
size_t map_size(const HashMap *map);

// ---------------------------------------------------
// Converters for generic map function arguments
//  these convert expressions to a MapKey or MapValue
// ---------------------------------------------------
MapKey key_for_long(long v);
MapKey key_for_double(double v);
MapKey key_for_string(const char *v);
MapKey key_for_void_ptr(const void *v);

MapValue value_for_long(long v);
MapValue value_for_double(double v);
MapValue value_for_string(const char *v);
MapValue value_for_void_ptr(const void *v);

// Macros for type-generic map functions
#define MAP_KEY(K) ( _Generic( (K), \
    float: key_for_double, \
    double: key_for_double, \
    long double: key_for_double, \
    char *: key_for_string,     const char *: key_for_string, \
    void *: key_for_void_ptr,   const void *: key_for_void_ptr, \
    default: key_for_long \
) (K) )

#define MAP_VALUE(V) ( _Generic( (V), \
    float: value_for_double, \
    double: value_for_double, \
    long double: value_for_double, \
    char *: value_for_string,     const char *: value_for_string, \
    void *: value_for_void_ptr,   const void *: value_for_void_ptr, \
    default: value_for_long \
) (V) )



#define map_contains_key(M, K) map_contains_key( (M), MAP_KEY(K))
#define map_contains_value(M, V) map_contains_value( (M), MAP_VALUE(V))
#define map_get(M, K) map_get( (M), MAP_KEY(K) )
#define map_get_or(M, K, V) map_get_or( (M), MAP_KEY(K), MAP_VALUE(V) )
#define map_try_get( M, K, V ) map_try_get( (M), MAP_KEY(K), V )
#define map_put(M, K, V) map_put( (M), MAP_KEY(K), MAP_VALUE(V) )
#define map_remove( M, K) map_remove( (M), MAP_KEY(K)  )
