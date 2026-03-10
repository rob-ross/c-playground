// array_list.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/09 17:30:57 PDT


#include "array_list.h"

#include <stdio.h>
#include <string.h>


const ListValuePolicy LIST_DEFAULT_VALUE_POLICY = (ListValuePolicy){
    .policy_type     = LIST_POLICY_NONE,
    .on_add_value    = nullptr,
    .on_free_value   = nullptr,
};

//// ------------------------------------------------------------
////
////    Public API methods
////
//// ------------------------------------------------------------


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
    //todo use clear()
    ListElement * temp = list->elements;
    printf("list->elements = %p, temp = %p\n", list->elements, temp);
    list->elements = nullptr;

    mem_free_bytes(list->mem_policy, temp);

    printf("list->elements = %p, temp = %p\n", list->elements, temp);

    mem_free_bytes(list->mem_policy, list);
}