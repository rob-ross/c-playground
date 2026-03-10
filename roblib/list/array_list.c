// array_list.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/09 17:30:57 PDT


#include "array_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static constexpr size_t MAX_POW2 = (SIZE_MAX >> 1) + 1;

const ListValuePolicy LIST_DEFAULT_VALUE_POLICY = (ListValuePolicy){
    .policy_type     = LIST_POLICY_NONE,
    .on_add_value    = nullptr,
    .on_free_value   = nullptr,
};

static void list_policy_value_free_default(List list[static 1], ListValue value) {
    if (!list) return;
    ListPolicyType valuepolicy = list->value_policy.policy_type;
    if ( value.value_type == LIST_TYPE_STRING &&
        ( valuepolicy == LIST_POLICY_COPY || valuepolicy == LIST_POLICY_TAKE || valuepolicy == LIST_POLICY_NONE )) {
        mem_free_bytes(list->mem_policy, value.vstring);  //we own it.
        } else if ( value.value_type == LIST_TYPE_VOID_PTR ) {
            // invoke the pointer's free method
            //todo deal with void* types
        }
}

static void list_free_value_if(List list[static 1], const ListElement element) {
    if (!list) return;
    if (list->value_policy.on_free_value) {
        list->value_policy.on_free_value(list, element.value);
    } else {
        list_policy_value_free_default(list, element.value);
    }
}



/**
 * Ensures this Vector has the wanted_capacity specified in the argument `wanted_capacity`. If the current wanted_capacity is less than the
 * argument, expands the Vector by doubling in size until the Vector wanted_capacity >= `wanted_capacity`.
 * @param list the List to check
 * @param wanted_capacity the desired capacity size
 * @return VEC_OK on success, or an error code on failure
 */
[[maybe_unused]]
static CollectionsError list_ensure_capacity(List list[static 1], const size_t wanted_capacity) {
    if (wanted_capacity > MAX_POW2) {
        return COL_ERR_OVERFLOW; // Can't grow more than MAX_POW2 * 2
    }

    if ( wanted_capacity >= list->capacity ) {
        // must grow buffer
        size_t new_capacity = list->capacity ? list->capacity * 2 : 1;

        do {
            new_capacity *= 2;  // keep doubling wanted_capacity until we can accomodate argument wanted_capacity
        } while (new_capacity < wanted_capacity);

        // Not sure why this is comparing to SIZE_MAX / sizeof(int). The check for
        // capacity > SIZE_MAX /2 is the first check in this function, so that's redundant here
        // if (list->capacity > SIZE_MAX / 2 || new_capacity > SIZE_MAX / sizeof(int)) {
        //     return COL_ERR_OVERFLOW;
        // }

        ListElement *re_ptr = mem_realloc_bytes(
            list->mem_policy, list->elements,
            list->capacity * sizeof(ListElement),  new_capacity * sizeof(ListElement)
        );
        if (!re_ptr) {
            return COL_ERR_OUT_OF_MEM;
        }
        list->elements = re_ptr;
        list->capacity = new_capacity;
    }
    return COL_OK;
}

//// ------------------------------------------------------------
////
////    Public API methods
////
//// ------------------------------------------------------------

CollectionsError list_append(List list[static 1], ListValue value) {
    if (!list) return COL_ERR_NULL_ARG;

    CollectionsError result = list_ensure_capacity(list, list->size);
    if (result) return result;

    list->elements[list->size] = (ListElement){ .value = value};
    list->size++;

    return COL_OK;
}


// Frees memory associated with each list element per the value and memory policies and sets size to 0.
// clears each index element using {}.
// Does not reduce the currently allocated capacity.
// todo some kind of resize method to realloc to a smaller memory footprint?
void list_clear(List list[static 1]) {
    const size_t size = list->size;
    for (size_t i = 0; i < size; i++) {
        list_free_value_if(list, list->elements[i]);
        list->elements[i] = (ListElement){}; // clear the element
    }
    list->size = 0;
}


// Returns nullptr on failure.
List * (list_create)( size_t capacity, ListValuePolicy value_policy, MemPolicy mem_policy) {

    capacity = (capacity == 0 ? 16 : capacity ); // min capacity is 16

    List *list = (List *)mem_alloc_bytes(mem_policy, sizeof(List));

    if (list == nullptr) {
        return nullptr;
    }

    ListElement *elements = (ListElement *)mem_alloc_bytes(mem_policy, capacity * sizeof(ListElement *));
    if (!elements) {
        mem_free_bytes(mem_policy, list);
        return nullptr;
    }

    List prototype = (List){
        .elements = elements,
        .size = 0,
        .capacity = capacity,
        .value_policy = value_policy,
        .mem_policy  = mem_policy,
        .flags = 0 };

    memcpy(list, &prototype, sizeof(List));
    return list;
}

void list_destroy(List list[static 1]) {
    if (!list) return;

    list_clear(list);

    // free any value policy contexts that exist
    if (list->value_policy.on_free_context) {
        list->value_policy.on_free_context(list->value_policy.context);
    }
    list->value_policy.context = nullptr;

    ListElement * elements = list->elements;
    printf("list->elements = %p, temp = %p\n", list->elements, elements);
    list->elements = nullptr;

    mem_free_bytes(list->mem_policy, elements);
    MemPolicy mem_policy = list->mem_policy;


    printf("list->elements = %p, temp = %p\n", list->elements, elements);

    mem_free_bytes(list->mem_policy, list);

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

ListValue list_get(const List list[static 1], const size_t index) {
    //todo bounds checking
    return list->elements[index].value;
}


bool list_is_empty(const List list[static 1]) {
    return list->size == 0;
}

size_t list_size(const List *list) {
    return list->size;
}
