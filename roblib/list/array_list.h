// array_list.h
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/09 17:30:57 PDT

//
// A dynamic list backed by a C-array.
//

#pragma once

#include "../memory/memory_pool.h"


// like HashMap, we first support long, double, string, and void*.
// like HashMap, the base List implementation is heterogenius
typedef enum ListTypeEnum: unsigned char {
    LIST_TYPE_NONE,
    LIST_TYPE_LONG,
    LIST_TYPE_DOUBLE,
    LIST_TYPE_STRING,
    LIST_TYPE_VOID_PTR,
    LIST_TYPE_NULL
} ListTypeEnum;

typedef struct ListValue {
    union {
        long   vlong;
        double vdouble;
        char  *vstring;
        void  *vvoid_ptr;
    };
    ListTypeEnum  value_type;
} ListValue;

struct List;

// defines we treat values passed to the List to store.
typedef enum ListPolicyType: uint8_t {
    LIST_POLICY_NONE, // default ininitialized value
    LIST_POLICY_COPY, // List makes a copy and owns the copy. List frees owned copy
    LIST_POLICY_TAKE, // List takes ownership and does not make a copy. List frees
    //todo must implement reference counting for shared policy
    LIST_POLICY_SHARED // List uses value, does not copy,  does not free.
} ListPolicyType;

typedef struct ListValuePolicy {
    // A context pointer to be passed to the policy functions.
    void* context;
    // Called when a value is added. Returns the value to be stored.
    // Can be used for copying, interning, or reference counting.
    ListValue (*on_add_value)(struct List *list, ListValue value);
    // Called when a value is freed.
    void (*on_free_value)(struct List *list, ListValue value);
    // for any specialized cleanup of the context
    void (*on_free_context)(void* context);
    ListPolicyType policy_type;
} ListValuePolicy;

extern const ListValuePolicy LIST_DEFAULT_VALUE_POLICY;

typedef struct ListElement {
    ListValue   value;
} ListElement;


typedef struct List {
    ListElement *elements;
    size_t size;     //the number of elements in this list.
    size_t capacity; // max number of elements this List can hold before needed to resize
    
    ListValuePolicy value_policy;
    MemPolicy       mem_policy;
    uint64_t flags; // future use
} List;


extern const MemPolicy LIST_DEFAULT_MALLOC_POLICY;
extern const ListValuePolicy LIST_DEFAULT_VALUE_POLICY;

// ---------------------------
// Public API methods
// ---------------------------

// adds the value to the end of the list
void list_append(List list[static 1], ListValue value) ;

//Removes all the elements from this list. After call, size == 0.
void list_clear(List list[static 1]);

// Returns true if the list contains the value, otherwise returns false.
bool list_contains(List list[static 1], ListValue value);
// call list_destroy to free all resources
List * (list_create)(size_t capacity, ListValuePolicy value_policy, MemPolicy mem_policy) ;
void list_destroy(List list[static 1]);

// Returns the element at the specified position in this list.
ListValue list_get(const List list[static 1], size_t index);

void list_insert(List list[static 1], ListValue value, size_t index);
bool list_is_empty(const List list[static 1]);

void list_remove(List list[static 1], size_t index);

// Returns the number of elements in this List
size_t list_size(const List *list);
//// ---------------------------------------------
////  repr methods
//// ---------------------------------------------
void list_repr_List(const List list[static 1], bool verbose, const char* type_str);