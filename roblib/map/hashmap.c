#include "hashmap.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> // Required for memcpy

// forward references
static bool doubles_are_equal(double a, double b);
static size_t hash_string(const char *str);
static size_t hash_mix64(size_t x);
static void resize_map(HashMap *map);

// ---------------------------
// static methods
// ---------------------------


// Compare the arguments for equality. Assumes k1 and k2 are of the same type.
static bool are_equal(const MapKey k1, const MapKey k2) {

    if (k1.key_type != k2.key_type) return false;

    switch (k1.key_type) {
        case MAP_TYPE_NONE:
            return false;
        case MAP_TYPE_LONG:
            return k1.klong == k2.klong;
        case MAP_TYPE_DOUBLE:
            return doubles_are_equal(k1.kdouble, k2.kdouble);
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

static inline size_t calc_bucket_index(const size_t hashcode, const size_t num_buckets) {
    return hashcode & (num_buckets - 1);  // works because num_buckets is a power of 2
}

static Node * create_node(const size_t hashcode, const MapKey key) {
    // hash and key are const, so we can't write to them after object is created. So we first create a temp Node
    // with the immutable values, then we set the mutable values, and finally we copy the temp to the final Node object.
    // for strings we make a copy and store that, since we will free it when the Node is removed or the map is
    // deleted. This is a temporary fix. Theres a whole copy/move design to consider, ultimate memory management,
    // etc. For the void* which represents some unknown blob of data somewhere, we may want to define a basic object
    // datatype, e.g. struct BasicObject, that has a pointer, a size, and a type. For flexibility type will probably
    // be a small string array of fixed size. Maybe the BasicObject will use a flexible array for the bytes. We can
    // use flags to indicate ownership? Do we free or not? And if so we'll need a funct ptr for a free function.
    Node *temp_node;
    switch (key.key_type) {
        case MAP_TYPE_LONG:
            temp_node = &(Node){ .hash = hashcode, .key.klong = key.klong, .key.key_type = MAP_TYPE_LONG };
            break;
        case MAP_TYPE_DOUBLE:
            temp_node = &(Node){ .hash = hashcode, .key.kdouble = key.kdouble, .key.key_type = MAP_TYPE_DOUBLE };
            break;
        case MAP_TYPE_STRING: {
            char *string_copy = strdup(key.kstring);
            temp_node = &(Node){ .hash = hashcode, .key.kstring = string_copy, .key.key_type = MAP_TYPE_STRING };
            break;
        }
        case MAP_TYPE_VOID_PTR:
            temp_node = &(Node){ .hash = hashcode, .key.kvoid_ptr = key.kvoid_ptr, .key.key_type = MAP_TYPE_VOID_PTR };
            break;
        case MAP_TYPE_NONE:
        default:
            temp_node = &(Node){};
            break;
    }

    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == nullptr) {
        // Handle allocation failure (in a real app, maybe return status)
        return nullptr;
    }
    memcpy(new_node, temp_node, sizeof(Node));

    return new_node;

}

static bool doubles_are_equal(double a, double b) {
    // Optional policy: NaNs are not equal to anything (including NaN)
    if (isnan(a) || isnan(b)) return false;

    // Make -0.0 and +0.0 compare equal (matches common hashing policies)
    if (a == 0.0) a = 0.0;
    if (b == 0.0) b = 0.0;

    uint64_t ua, ub;
    memcpy(&ua, &a, sizeof ua);
    memcpy(&ub, &b, sizeof ub);
    return ua == ub;
}

static void free_if(const Node *node_ptr, const HashMap *map) {
    if ( node_ptr->value.value_type == MAP_TYPE_STRING) {
        map->free_func(node_ptr->value.vstring);
    }
    else if ( node_ptr->value.value_type == MAP_TYPE_VOID_PTR) {
        map->free_func(node_ptr->value.vvoid_ptr);
    }
}

static size_t hash_function(const MapKey key) {
    size_t raw_hash;
    switch (key.key_type) {
        case MAP_TYPE_LONG: {
            // For integer keys, especially sequential ones, we multiply by a large
            // prime to spread the bits out across the 64-bit range. This ensures
            // the subsequent mixer works effectively. 0x9e3779b97f4a7c15 is a
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
            raw_hash = hash_string((char*)key.kstring);
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
    return hash_mix64(raw_hash);
}

// hash mixer
static size_t hash_mix64( size_t x ) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

//djb2 hash algorithm, O(N) (actually Theta(N))
static size_t hash_string(const char *str) {
    unsigned long hash = 5381; // A "magic" prime number
    int c;

    while (  (c = (unsigned char)(*str++) ) ) {
        // hash = (hash * 33) + c
        // This is a fast way to write it using bit shifts:
        hash = ((hash << 5) + hash) + c;
    }

    return (size_t)hash;
}

static constexpr size_t MIN_CAP  = 16;
static constexpr size_t MAX_POW2 = (SIZE_MAX >> 1) + 1;

static size_t next_power_of_two(size_t n) {
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

static void recalc_load(HashMap *map) {
    map->load =  (double) ((long double)map->size / map->num_buckets);
}

static void resize_map(HashMap *map) {
    const size_t new_num_buckets = map->num_buckets * 2;
    if (new_num_buckets > MAX_POW2) {
        return; // Can't grow anymore
    }

    Node **new_buckets = (Node **)calloc(new_num_buckets, sizeof(Node *));
    if (!new_buckets) {
        return; // Allocation failed, keep old map
    }

    // Rehash all existing nodes
    for (size_t i = 0; i < map->num_buckets; i++) {
        Node *current = map->buckets[i];
        while (current != nullptr) {
            Node *next = current->next; // Save next pointer

            // Calculate new index
            size_t new_index = calc_bucket_index(current->hash, new_num_buckets);

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
    recalc_load(map);
}


static void set_value(Node *node, const MapValue value) {
    switch (value.value_type) {
        case MAP_TYPE_LONG:
            node->value.vlong = value.vlong;
            break;
        case MAP_TYPE_DOUBLE:
            node->value.vdouble = value.vdouble;
            break;
        case MAP_TYPE_STRING:
            node->value.vstring = strdup(value.vstring);
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




// ---------------------------
// API methods
// ---------------------------

// returns nullptr on failure. if num_buckets == 0, uses 16 as initial bucket size.
// num_buckets is clamped to smalles power of two > num_buckets.
// buckets double when fill capacity is reached (75% full). 16 buckets provides adequate sizing for 12 items before
// doubling.
HashMap *create_map(size_t num_buckets, void (*free_value_func)(void *)) {
    HashMap *map = (HashMap *)malloc(sizeof(HashMap));
    if (map == nullptr) {
        return nullptr;
    }
    if (!num_buckets) {
        num_buckets = 16;
    } else if ( num_buckets > (SIZE_MAX >> 1) + 1) {
       num_buckets = (SIZE_MAX >> 1) + 1;
    } else {
        num_buckets = next_power_of_two(num_buckets);
    }
    map->buckets = (Node **)calloc(num_buckets, sizeof(Node *));
    if (map->buckets == nullptr) {
        free(map);
        return nullptr;
    }

    map->num_buckets = num_buckets;
    map->size = 0;
    map->fill_factor = DEFAULT_FILL_FACTOR;
    map->load = 0;
    map->free_func = free_value_func;
    map->fill_capacity = (size_t)( num_buckets * (long double)map->fill_factor );

    return map;
}

// deletes all entries and frees them, but does not reduce bucket size or free allocated bucket memory.
// todo some kind of resize method to realloc to a smaller memory footprint?
void map_clear(HashMap map[static 1]) {
    for (size_t i = 0; i < map->num_buckets; i++) {
        Node *current = map->buckets[i];
        while (current != nullptr) {
            Node *temp = current;
            current = current->next;
            free_if(temp, map);
            free(temp);
        }
    }
    map->load = 0;
    map->size = 0;
}
bool (map_contains_key)(HashMap map[static 1], const MapKey key) {
    MapValue unused;
    return map_try_get(map, key, &unused);
}
void (map_remove)(HashMap map[static 1], const MapKey key) {
    if (!map) return;

    const size_t hashcode = hash_function(key);
    const size_t index = calc_bucket_index(hashcode, map->num_buckets);

    Node *current = map->buckets[index];
    Node *prev = nullptr;

    while (current) {
        if ( are_equal(key, current->key ) ){
            if (!prev) {
                map->buckets[index] = current->next;
            } else {
                prev->next = current->next;
            }
            free_if(current, map);
            free(current);
            map->size--;
            recalc_load(map);
            return;
        }
        prev = current;
        current = current->next;

    }
}

void free_map(HashMap map[static 1]) {
    if (map == nullptr) return;

    for (size_t i = 0; i < map->num_buckets; i++) {
        Node *current = map->buckets[i];
        while (current != nullptr) {
            Node *temp = current;
            current = current->next;
            free_if(temp, map);
            free(temp);
        }
    }

    free(map->buckets);
    free(map);
}


MapValue (map_get)(const HashMap map[static 1], const MapKey key) {
    if (map == nullptr) return NULL_MAP_VALUE;
    const size_t index = calc_bucket_index(hash_function(key), map->num_buckets);

    Node const *current = map->buckets[index];

    while (current != nullptr) {
        if ( are_equal(key, current->key) ) {
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

bool map_try_get(const HashMap map[static 1], const MapKey key, MapValue *out) {
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
        resize_map(map);
    }

    const size_t hashcode = hash_function(key);
    const size_t bucket_index = calc_bucket_index(hashcode, map->num_buckets);
    Node *current = map->buckets[bucket_index];

    // Check if key already exists and update value
    while (current != nullptr) {
        if ( are_equal(key, current->key) ) {
            free_if(current, map);
            set_value(current, value);
            return;
        }
        current = current->next;
    }
    // Key not found, insert new node at the beginning of the list
    Node *new_node = create_node(hashcode, key);
    if (new_node == nullptr) {
        // Handle allocation failure (in a real app, maybe return status)
        return;
    }
    new_node->next =  map->buckets[bucket_index];  // inserts this Node at the head of the bucket
    set_value(new_node, value);
    map->buckets[bucket_index] = new_node;
    map->size++;
    recalc_load(map);

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

void map_repr_Node(const Node node[static 1]) {
    if (!node) {
        printf("(Node)nullptr");
        return;
    }
    printf("(Node){ ");
    printf(".hash=0x%-16lX", node->hash);
    printf(", .this=%14p, .next=%14p, ", node, node->next);
    map_repr_MapKey(node->key, false);
    printf(", ");
    map_repr_MapValue(node->value, false);
    printf(" }\n");
}

//display info about the HashMap via println
void map_repr_HashMap(const HashMap map[static 1], bool verbose) {
    if (!map) {
        printf("(HashMap)nullptr");
        return;
    }

    // ReSharper disable CppPrintfBadFormat
    // ReSharper disable CppPrintfExtraArg
    printf( "(HashMap){ .size=%'zu, .fill_capacity=%'zu, .load=%'g, .num_buckets=%'zu, .fill_factor=%g, .free_func=%p }",
            map->size, map->fill_capacity, map->load, map->num_buckets, map->fill_factor, map->free_func);

    const int max_buckets_displayed = 16;
    // we might want a special Bucket struct to be the head of each bucket, so we can keep statistics on the bucket
    // contents
    // For now we'll do it the hard way and generate stats on the fly by visiting every Node in the HashMap
    printf("\n");
    if (verbose) {
        size_t total_bucket_items = 0;
        printf("bucket sizes: ");
        for (size_t i=0; i < map->num_buckets; ++i) {
            const Node *current = map->buckets[i];
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
            const Node *current = map->buckets[i];
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

// ---------------------------------------------------
// Converters for generic map function arguments
//  these convert expressions to a MapKey or MapValue
// ---------------------------------------------------

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

MapKey key_for_long(const long v) {
    return (MapKey){.klong = v, .key_type = MAP_TYPE_LONG};
}

MapKey key_for_double(const double v) {
    return (MapKey){.kdouble = v, .key_type = MAP_TYPE_DOUBLE};
}

MapKey key_for_string(const char * v) {
    return (MapKey){.kstring = (char*)v, .key_type = MAP_TYPE_STRING};
}

MapKey key_for_void_ptr(const void * v) {
    return (MapKey){.kvoid_ptr = (void*)v, .key_type = MAP_TYPE_VOID_PTR};
}
