#include "hashmap.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> // Required for memcpy

static bool doubles_are_equal(double a, double b);
static size_t hash_string(const char *str);
static size_t hash_mix64(size_t x);
static void resize_map(HashMap *map);

// ---------------------------
// static methods
// ---------------------------

static void * address_of_map_value(Node *node) {
    if (node->value.value_type == MAP_TYPE_STRING) {
        return node->value.vstring;
    }
    if (node->value.value_type == MAP_TYPE_VOID_PTR) {
        return node->value.vvoid_ptr;
    }
    if (node->value.value_type == MAP_TYPE_LONG) {
        return &node->value.vlong;
    }
    if (node->value.value_type == MAP_TYPE_DOUBLE) {
        return &node->value.vdouble;
    }
    return nullptr;
}

// Compare the arguments for equality. Assumes o1 and o2 are of the same type.
static bool are_equal(const void *o1, const void *o2, const MapTypeEnum key_type) {
    switch (key_type) {
        case MAP_TYPE_NONE:
            return false;
        case MAP_TYPE_LONG: {
            const long long1 = *(long *)o1;
            const long long2 = *(long *)o2;
            return long1 == long2;
        }

        case MAP_TYPE_DOUBLE: {
            const double d1 = *(double*)o1;
            const double d2 = *(double*)o2;
            return doubles_are_equal(d1, d2);
        }

        case MAP_TYPE_STRING: {
            const char* str1 = (char*)o1;
            const char* str2 = (char*)o2;
            return strcmp(str1, str2) == 0;
        }

        case MAP_TYPE_VOID_PTR:
            // this requires the caller to have defined an equal function for this blob.
            return false; // todo implement
        default:
            return false;
    }
}

static inline size_t calc_bucket_index(const size_t hashcode, const size_t num_buckets) {
    return hashcode & (num_buckets - 1);  // works because num_buckets is a power of 2
}

static Node * create_node(const size_t hashcode, const void *key, const MapTypeEnum key_type ) {
    // hash and key are const, so we can't write to them after object is created. So we first create a temp Node
    // with the immutable values, then we set the mutable values, and finally we copy the temp to the final Node object.
    // for strings we make a copy and store that, since we will free it when the Node is removed or the map is
    // deleted. This is a temporary fix. Theres a whole copy/move design to consider, ultimate memory management,
    // etc. For the void* which represents some unknown blob of data somewhere, we may want to define a basic object
    // datatype, e.g. struct BasicObject, that has a pointer, a size, and a type. For flexibility type will probably
    // be a small string array of fixed size. Maybe the BasicObject will use a flexible array for the bytes. We can
    // use flags to indicate ownership? Do we free or not? And if so we'll need a funct ptr for a free function.

    //
    Node *temp_node;
    switch (key_type) {
        case MAP_TYPE_LONG:
            temp_node = &(Node){ .hash = hashcode, .key.klong   = *(long *)key,   .key.key_type = MAP_TYPE_LONG };
            break;
        case MAP_TYPE_DOUBLE:
            temp_node = &(Node){ .hash = hashcode, .key.kdouble = *(double *)key, .key.key_type = MAP_TYPE_DOUBLE };
            break;
        case MAP_TYPE_STRING: {
            char *string_copy = strdup((char *)key);
            temp_node = &(Node){ .hash = hashcode, .key.kstring = string_copy,    .key.key_type = MAP_TYPE_STRING };
            break;
        }
        case MAP_TYPE_VOID_PTR:
            temp_node = &(Node){ .hash = hashcode, .key.kvoid_ptr = (void *)key,    .key.key_type = MAP_TYPE_VOID_PTR };
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

static const void * get_mapkey_member_ptr(const MapKey *mp, const MapTypeEnum key_type) {
    const void *node_key_ptr;
    switch (key_type) {
        case MAP_TYPE_LONG:    node_key_ptr = &mp->klong;    break;
        case MAP_TYPE_DOUBLE:  node_key_ptr = &mp->kdouble;  break;
        case MAP_TYPE_STRING:  node_key_ptr = mp->kstring;   break; // note: string key is already a pointer
        case MAP_TYPE_VOID_PTR:node_key_ptr = mp->kvoid_ptr; break; // also a pointer
        default:               node_key_ptr = nullptr;       break;
    }
    return node_key_ptr;
}

// Simple hash function for long keys
static size_t hash_function(const void *key, const MapTypeEnum key_type) {
    size_t raw_hash;
    switch (key_type) {
        case MAP_TYPE_LONG: {
            // For integer keys, especially sequential ones, we multiply by a large
            // prime to spread the bits out across the 64-bit range. This ensures
            // the subsequent mixer works effectively. 0x9e3779b97f4a7c15 is a
            // common choice related to the golden ratio.
            raw_hash =  (*(long *)key) * 0x9e3779b97f4a7c15ULL;
            break;
        }
        case MAP_TYPE_DOUBLE: {
            double d = *(double *)key;
            // Normalize -0.0 to 0.0 so they hash to the same bucket
            if (d == 0.0) d = 0.0;
            size_t hash = 0;
            if (sizeof(size_t) >= sizeof(double)) {
                memcpy(&hash, &d, sizeof(double));
            } else {
                unsigned long long bits;
                memcpy(&bits, &d, sizeof(double));
                hash = (size_t)(bits ^ (bits >> 32));
            }
            raw_hash = hash;
            break;
        }
        case MAP_TYPE_STRING: {
            raw_hash = hash_string((char*)key);
            break;
        }
        case MAP_TYPE_VOID_PTR: {
            // this requires the caller to have defined a hash function for this blob.
            raw_hash = (unsigned long)key;
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
    size_t new_num_buckets = map->num_buckets * 2;
    if (new_num_buckets > MAX_POW2) {
        return; // Can't grow anymore
    }

    Node **new_buckets = (Node **)calloc(new_num_buckets, sizeof(Node *));
    if (new_buckets == nullptr) {
        return; // Allocation failed, keep old map
    }

    repr_HashMap(map, false);

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

    repr_HashMap(map, false);
}

static void set_value(Node *node, const void *value, const MapTypeEnum value_type) {
    switch (value_type) {
        case MAP_TYPE_NONE:
            node->value.vvoid_ptr = nullptr;
            break;
        case MAP_TYPE_STRING:
            node->value.vstring = strdup((char*)value);
            break;
        case MAP_TYPE_VOID_PTR:
            node->value.vvoid_ptr = (void*)value;
            break;
        case MAP_TYPE_DOUBLE:
            node->value.vdouble = *((double *)value);
            break;
        case MAP_TYPE_LONG:
            node->value.vlong = *((long *)value);
            break;
    }
    node->value.value_type = value_type;
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

void delete_key(HashMap *map, void *key, const MapTypeEnum key_type) {
    if (map == nullptr) return;

    const size_t hashcode = hash_function(key, key_type);
    const size_t index = calc_bucket_index(hashcode, map->num_buckets);

    Node *current = map->buckets[index];
    Node *prev = nullptr;

    while (current != nullptr) {
        if ( current->key.key_type == key_type) {
            if ( are_equal(key, get_mapkey_member_ptr(&current->key, current->key.key_type), current->key.key_type) ) {
                if (prev == nullptr) {
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
}

void free_map(HashMap *map) {
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

void * get(HashMap *map, const void *key, const MapTypeEnum key_type, const MapTypeEnum value_type) {
    if (map == nullptr) return nullptr;
    const size_t index = calc_bucket_index(hash_function(key, key_type), map->num_buckets);

    Node *current = map->buckets[index];

    while (current != nullptr) {
        if ( current->key.key_type == key_type) {
            if ( are_equal(key, get_mapkey_member_ptr(&current->key, current->key.key_type), current->key.key_type) ) {
                return address_of_map_value(current);
            }
        }
        current = current->next;
    }
    return nullptr; // Key not found
}

// if key or value are strings, they are copied so the map can be free them independently of the original arguments.
void put(HashMap *map, const void *key, const void *value, const MapTypeEnum key_type, const MapTypeEnum value_type) {
    if (map == nullptr) return;

    if (map->size >= map->fill_capacity) {
        resize_map(map);
    }

    const size_t hashcode = hash_function(key, key_type);
    const size_t bucket_index = calc_bucket_index(hashcode, map->num_buckets);
    Node *current = map->buckets[bucket_index];

    // Check if key already exists and update value
    while (current != nullptr) {
        // First, check if the key types even match. If not, they can't be equal.
        if ( current->key.key_type == key_type) {
            if ( are_equal(key, get_mapkey_member_ptr(&current->key, current->key.key_type), current->key.key_type) ) {
                free_if(current, map);
                set_value(current, value, value_type);
                return;
            }
        }
        current = current->next;
    }
    // Key not found, insert new node at the beginning of the list
    Node *new_node = create_node(hashcode, key, key_type);
    if (new_node == nullptr) {
        // Handle allocation failure (in a real app, maybe return status)
        return;
    }
    new_node->next =  map->buckets[bucket_index];  // inserts this Node at the head of the bucket
    set_value(new_node, value, value_type);
    map->buckets[bucket_index] = new_node;
    map->size++;
    recalc_load(map);
}

void repr_MapValue(const MapValue *map_value, const bool verbose) {
    if (!map_value) {
        printf("(MapValue)nullptr");
        return;
    }
    if (map_value->value_type == MAP_TYPE_NONE) {
        printf("(MapValue){ MAP_TYPE_NONE }");
        return;
    }

    if (verbose) {
        printf("(MapValue){ ");
    }

    switch (map_value->value_type) {
        case MAP_TYPE_LONG:
            printf(".vlong=%5lu", map_value->vlong);
            break;
        case MAP_TYPE_DOUBLE:
            printf(".vdouble=%5g", map_value->vdouble);
            break;
        case MAP_TYPE_STRING:
            printf(".vstring='%5s'", map_value->vstring);
            break;
        case MAP_TYPE_VOID_PTR:
            printf(".vvoid_ptr=%14p", map_value->vvoid_ptr);
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

void repr_MapKey(const MapKey *map_key, bool verbose) {
    if (!map_key) {
        printf("(MapKey)nullptr");
        return;
    }

    if (map_key->key_type == MAP_TYPE_NONE) {
        printf("(MapKey){ MAP_TYPE_NONE }");
        return;
    }

    if (verbose) {
        printf("(MapKey){ ");
    }

    switch (map_key->key_type) {
        case MAP_TYPE_LONG:
            printf(".klong=%5lu", map_key->klong);
            break;
        case MAP_TYPE_DOUBLE:
            printf(".kdouble=%5g", map_key->kdouble);
            break;
        case MAP_TYPE_STRING:
            printf(".kstring='%5s'", map_key->kstring);
            break;
        case MAP_TYPE_VOID_PTR:
            printf(".kvoid_ptr=%14p", map_key->kvoid_ptr);
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

void repr_node(const Node *node) {
    if (!node) {
        printf("(Node)nullptr");
        return;
    }
    printf("(Node){ ");
    printf(".hash=%20zu", node->hash);
    printf(", .this=%14p, .next=%14p, ", node, node->next);
    repr_MapKey(&node->key, false);
    printf(", ");
    repr_MapValue(&node->value, false);
    printf(" }\n");
}

//display info about the HashMap via println
void repr_HashMap(const HashMap *map, bool verbose) {
    if (!map) {
        printf("(HashMap)nullptr");
        return;
    }
    printf( "(HashMap){ .size=%'zu, .fill_capacity=%'zu, .load=%'g, .num_buckets=%'zu, .fill_factor=%g, .free_func=%p }",
        map->size, map->fill_capacity, map->load, map->num_buckets, map->fill_factor, map->free_func);

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
            while (current != nullptr) {
                node_count++;
                current = current->next;
            }
            total_bucket_items += node_count;
            printf("[%zu]=%zu, ", i, node_count);
        }
        printf("\n");
        printf("total bucket items: %zu\n\n", total_bucket_items);


        // for each bucket, display name/value pairs.
        for (size_t i=0; i < map->num_buckets; ++i) {
            const Node *current = map->buckets[i];
            printf("bucket[%zu]:",i);
            if (!current) {
                printf(" empty\n");
            } else {
                printf("\n");
            }
            size_t node_count = 0;
            while (current != nullptr) {
                node_count++;
                repr_node(current);
                current = current->next;
            }
            printf("---------------------------------------------------------------------------------------------------------------------\n");
            printf("bucket[%zu] node count:%zu\n",i,node_count);
            printf("\n");
        }
    }

}