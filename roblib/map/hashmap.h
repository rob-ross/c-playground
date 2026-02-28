

#pragma once

#include <stddef.h> // For size_t
#include <stdint.h> // For uint8_t

// Define a Node for the linked list within each bucket

//we will start by supporting scalar keys. For int values we'll support signed/unsigned int and long.
// for floats we'll support double.
// we get char via int
// we also support string keys
// we can support arbitrary objects via a void* key, but we'll save that for later
// for values we should support all the the scalar types, which we can do via int/long and double.
// for values we will also support char* specifically for strings, and void* for arbitrary data.

// in phase 2 we'll want to be able to expand the size of the hashmap as the contents grow
// in phase 3 we'll optimize memory by allocating in chunks so we can keep our buckets and linked lists in the same
// memory chunk.



typedef enum MapTypeEnum: unsigned char {
    MAP_TYPE_NONE,
    MAP_TYPE_LONG,
    MAP_TYPE_DOUBLE,
    MAP_TYPE_STRING,
    MAP_TYPE_VOID_PTR,
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
typedef struct MapKey {
    union {
        long klong;
        double kdouble;
        char *kstring;
        void *kvoid_ptr;
    };
    MapTypeEnum  key_type;
} MapKey;

typedef struct MapValue {
    union {
        long   vlong;
        double vdouble;
        char  *vstring;
        void  *vvoid_ptr;
    };
    MapTypeEnum  value_type;
} MapValue;


typedef struct Node {
    const MapKey key;
    MapValue     value;
    const size_t hash;
    struct Node  *next;
} Node;

static const Node NULL_NODE = { };
static constexpr double DEFAULT_FILL_FACTOR = 0.75;

// Define the HashMap structure
typedef struct HashMap {
    Node **buckets;       // each bucket is a linked list
    size_t size;          // Number of key-value pairs currently in the map
    size_t fill_capacity; // holds the max ideal size for the current number of buckets. Increases when buckets increase
    double load;          // current load = size / num_buckets
    size_t num_buckets;   // must always be a power of 2
    double fill_factor;   // desired load
    void (*free_func)(void *); // Function pointer to free allocated values
    uint64_t flags; // future use
} HashMap;


// Function prototypes
HashMap *create_map(size_t num_buckets, void (*free_value_func)(void *));
//Removes all of the mappings from this map
void clear(void);
//  Returns true if this map contains a mapping for the specified key.
bool contains_key(void *key);
//  Returns true if this map contains a mapping for the specified key.
bool contains_value(void *value);
//Removes the mapping for a key from this map if it is present
// Returns the value to which this map previously associated the key, or null if the map contained no mapping for the key.
void delete_key(HashMap *map, void *key, MapTypeEnum key_type);
void free_map(HashMap *map);
void *get(HashMap *map, const void *key, MapTypeEnum key_type, MapTypeEnum value_type) ;
// Returns the value to which the specified key is mapped, or fallback if this map contains no mapping for the key.
void *get_or(HashMap *map, const void *key, MapTypeEnum key_type, MapTypeEnum value_type, const void *fallback) ;
// Returns true if this map contains no key-value mappings.
bool is_empty(void);
void put(HashMap *map, const void *key, const void *value, MapTypeEnum key_type, MapTypeEnum value_type) ;
void repr_HashMap(const HashMap *map, bool verbose);
// Returns the number of key-value mappings in this map.
size_t size(void);


// Wrappers (type-safe-ish at compile time)

// -----------------------------------------
// put methods
// -----------------------------------------

// long key

static inline void map_put_klong_vlong(HashMap *m, long k, long v) {
    put(m, &k, &v, MAP_TYPE_LONG, MAP_TYPE_LONG);
}

static inline void map_put_klong_vdouble(HashMap *m, long k, double v) {
    put(m, &k, &v, MAP_TYPE_LONG, MAP_TYPE_DOUBLE);
}

static inline void map_put_klong_vstring(HashMap *m, long k, char *v) {
    put(m, &k, v, MAP_TYPE_LONG, MAP_TYPE_STRING);
}


// double key

static inline void map_put_kdouble_vlong(HashMap *m, double k, long v) {
    put(m, &k, &v, MAP_TYPE_DOUBLE, MAP_TYPE_LONG);
}

static inline void map_put_kdouble_vdouble(HashMap *m, double k, double v) {
    put(m, &k, &v, MAP_TYPE_DOUBLE, MAP_TYPE_DOUBLE);
}

static inline void map_put_kdouble_vstring(HashMap *m, double k, char *v) {
    put(m, &k, v, MAP_TYPE_DOUBLE, MAP_TYPE_STRING);
}

// string key
static inline void map_put_kstring_vlong(HashMap *m, char* k, long v) {
    put(m, k, &v, MAP_TYPE_STRING, MAP_TYPE_LONG);
}

static inline void map_put_kstring_vdouble(HashMap *m, char* k, double v) {
    put(m, k, &v, MAP_TYPE_STRING, MAP_TYPE_DOUBLE);
}

static inline void map_put_kstring_vstring(HashMap *m, char* k, char *v) {
    put(m, k, v, MAP_TYPE_STRING, MAP_TYPE_STRING);
}


// -----------------------------------------
// get methods : return nullptr if not found, else a pointer to the value. you must dereference it!
// -----------------------------------------

// these are problematic. The user must know what the type of the value is for the key ahead of time.
// this feels unrealistic. This is probably why most generic maps are homogenious
// todo refactor this DS. We'll evolve this version to accomodate multiple key types and values at the cost of
// some more work for the user. We'll have to return a struct with the value type and  the
// user will have to cast it. Actually, we can return a MapValue. We'll have to refactor it into a struct
// with the same union as a member, but with the type information as well, so it's a discriminated union.
// we'll pass a copy to the caller so they can't clobber the data.




// long key

static inline void *map_get_klong_vlong(HashMap *m, long k) {
    return get(m, &k, MAP_TYPE_LONG, MAP_TYPE_LONG);
}

static inline void *map_get_klong_vdouble(HashMap *m, long k) {
    return get(m, &k, MAP_TYPE_LONG, MAP_TYPE_DOUBLE);
}

static inline void *map_get_klong_vstring(HashMap *m, long k) {
    return get(m, &k, MAP_TYPE_LONG, MAP_TYPE_STRING);
}


// double key
static inline void *map_get_kdouble_vlong(HashMap *m, double k) {
    return get(m, &k, MAP_TYPE_DOUBLE,MAP_TYPE_LONG);
}

static inline void *map_get_kdouble_vdouble(HashMap *m, double k) {
    return get(m, &k, MAP_TYPE_DOUBLE, MAP_TYPE_DOUBLE);
}

static inline void *map_get_kdouble_vstring(HashMap *m, double k) {
    return get(m, &k, MAP_TYPE_DOUBLE,MAP_TYPE_STRING);
}

// string key
static inline long * map_get_kstring_vlong(HashMap *m, char *k) {
    return get(m, k, MAP_TYPE_STRING,MAP_TYPE_LONG);
}

static inline const char *map_get_kstring_vdouble(HashMap *m, char *k) {
    return get(m, k, MAP_TYPE_STRING, MAP_TYPE_DOUBLE);
}

static inline const char *map_get_kstring_vstring(HashMap *m, char *k) {
    return get(m, k, MAP_TYPE_STRING,MAP_TYPE_STRING);
}



// -----------------------------------------
// delete methods
// -----------------------------------------

static inline void map_delete_klong(HashMap *m, long k) {
    delete_key(m, &k, MAP_TYPE_LONG);
}

static inline void map_delete_kdouble(HashMap *m, double k) {
    delete_key(m, &k, MAP_TYPE_DOUBLE);
}

static inline void map_delete_kstring(HashMap *m, char *k) {
    delete_key(m, k, MAP_TYPE_STRING);
}