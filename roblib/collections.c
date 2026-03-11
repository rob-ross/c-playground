// collections.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/10 13:55:52 PDT

//
// Common objects shared by collection classes (HashMap, List)
//

#include "collections.h"

// -----------------------
// Value data_policies
// -----------------------

ColValue col_policy_value_set_default(void * context, ColValue value, ColValuePolicyType value_policy, MemPolicy mem_policy ) {
    // default add always makse a copy of a string key and we own it
    if ( value.value_type == COL_TYPE_STRING &&
        ( value_policy == COL_VALUE_POLICY_COPY || value_policy == COL_VALUE_POLICY_NONE )) {
        char *string_copy = mem_strdup(mem_policy, value.vstring);
        return (ColValue){.value_type = COL_TYPE_STRING, .vstring = string_copy};
    }
    if ( value.value_type == COL_TYPE_VOID_PTR ) {
        // invoke the pointer's add method?
        //todo deal with void* types
    }
    return value;
}

void col_policy_value_free_default(void * context, ColValue value, ColValuePolicyType value_policy, MemPolicy mem_policy ) {
    if ( value.value_type == COL_TYPE_STRING &&
        ( value_policy == COL_VALUE_POLICY_COPY || value_policy == COL_VALUE_POLICY_TAKE || value_policy == COL_VALUE_POLICY_NONE )) {
        mem_free_bytes(mem_policy, value.vstring);  //we own it.
    } else if ( value.value_type == COL_TYPE_VOID_PTR ) {
            // invoke the pointer's free method
            //todo deal with void* types
    }
}


size_t col_next_power_of_two(size_t wanted_size, const size_t min_size) {
    // Clamp lower bound
    if (wanted_size < min_size) return min_size;

    // Clamp upper bound
    if (wanted_size >= MAX_POW2) return MAX_POW2;

    // Round up to next power of two
    // algorithm:
    // -----------------------------
    // 1. Subtract 1 so exact powers of two don’t round up.
    // 2. Fill all bits below the highest 1 with 1s.
    // 3. Add 1.
    wanted_size--;  // clears the lowest set bit and turns all lower bits into 1.
    wanted_size |= wanted_size >> 1;
    wanted_size |= wanted_size >> 2;
    wanted_size |= wanted_size >> 4;
    wanted_size |= wanted_size >> 8;
    wanted_size |= wanted_size >> 16;
#if SIZE_MAX > 0xffffffff
    wanted_size |= wanted_size >> 32;
#endif
    return wanted_size + 1;
}


//// -----------------------------------------------------
////    Converters for generic map function arguments
////
////    these convert expressions to a ColValue
//// -----------------------------------------------------

ColValue value_for_long(const long v) {
    return (ColValue){.vlong = v, .value_type = COL_TYPE_LONG};
}

ColValue value_for_double(const double v) {
    return (ColValue){.vdouble = v, .value_type = COL_TYPE_DOUBLE};
}

ColValue value_for_string(const char * v) {
    return (ColValue){.vstring = (char*)v, .value_type = COL_TYPE_STRING};
}

ColValue value_for_void_ptr(const void * v) {
    return (ColValue){.vvoid_ptr = (void*)v, .value_type = COL_TYPE_VOID_PTR};
}