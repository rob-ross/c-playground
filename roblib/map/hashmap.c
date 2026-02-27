#include "hashmap.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> // Required for memcpy

static bool doubles_are_equal(double a, double b);
static size_t hash_string(const char *str);

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
    return hashcode % num_buckets;
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
        map->free_value(node_ptr->value.vstring);
    }
    else if ( node_ptr->value.value_type == MAP_TYPE_VOID_PTR) {
        map->free_value(node_ptr->value.vvoid_ptr);
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
    switch (key_type) {
        case MAP_TYPE_LONG:
            return (size_t)(*(long *)key);
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
            return hash;
        }
        case MAP_TYPE_STRING:
            return hash_string((char*)key);
        case MAP_TYPE_VOID_PTR:
            // this requires the caller to have defined a hash function for this blob.
            return (unsigned long)key;
        case MAP_TYPE_NONE:
        default:
            return 0;
    }
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

// returns nullptr on failure
HashMap *create_map(const size_t num_buckets, void (*free_value_func)(void *)) {
    HashMap *map = (HashMap *)malloc(sizeof(HashMap));
    if (map == nullptr) {
        return nullptr;
    }

    map->buckets = (Node **)calloc(num_buckets, sizeof(Node *));
    if (map->buckets == nullptr) {
        free(map);
        return nullptr;
    }

    map->num_buckets = num_buckets;
    map->size = 0;
    map->free_value = free_value_func;
    map->prime_index = 0;

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

void * get(HashMap *map, const void *key, const MapTypeEnum key_type) {
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
}