



#include "snekobject.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "../munit/munit.h"
#include "../munit/munit_overrides.h"
#include "../munit/memory_check.h"

// make clang snekobject.c ../munit/munit.c -o snekobject.out

snek_object_t *snek_add(snek_object_t *a, snek_object_t *b) {
    if (!a || ! b) {
        return NULL;
    }
    switch (a->kind) {
        case INTEGER:
            if (b->kind == INTEGER) {
                return new_snek_integer( a->data.v_int + b->data.v_int );
            }
            if (b->kind == FLOAT) {
                return new_snek_float( a->data.v_int + b->data.v_float );
            }
            return NULL;
        case FLOAT:
            if (b->kind == INTEGER) {
                return new_snek_float( a->data.v_float + b->data.v_int );
            }
            if (b->kind == FLOAT) {
                return new_snek_float( a->data.v_float + b->data.v_float );
            }
            return NULL;
        case STRING:
            if (b->kind == STRING) {
                int newlen = strlen(a->data.v_string) + strlen(b->data.v_string) + 1;
                char *ns = calloc(newlen, sizeof(char));
                if (!ns) {
                    return NULL;
                }
                strcat(ns, a->data.v_string);
                strcat(ns, b->data.v_string);
                snek_object_t *obj = new_snek_string(ns);
                free(ns);
                return obj;
            }
            return NULL;
        case VECTOR3:
            if (b->kind == VECTOR3) {
                snek_vector_t nv3 = {0};
                snek_vector_t av3 = a->data.v_vector3;
                snek_vector_t bv3 = b->data.v_vector3;
                nv3.x = snek_add(av3.x, bv3.x);
                nv3.y = snek_add(av3.y, bv3.y);
                nv3.z = snek_add(av3.z, bv3.z);
                snek_object_t *v3_p = new_snek_vector3(nv3.x, nv3.y, nv3.z);
                return v3_p;
            }
            return NULL;
        case ARRAY:
            if (b->kind == ARRAY) {
                snek_object_t *new_array = new_snek_array(a->data.v_array.size + b->data.v_array.size);
                for (size_t i = 0; i < a->data.v_array.size; ++i) {
                    snek_array_set(new_array, i, snek_array_get(a, i));
                }
                for (size_t i = 0; i < b->data.v_array.size; ++i) {
                    snek_array_set(new_array, i + a->data.v_array.size, snek_array_get(b, i));
                }
                return new_array;
            }
            return NULL;
    }
    return NULL;
}

int snek_length(snek_object_t *obj) {
    if (!obj) {
        return -1;
    }
    switch (obj->kind) {
        case INTEGER: return 1;
        case FLOAT:   return 1;
        case STRING:  return strlen(obj->data.v_string);
        case VECTOR3: return 3;;
        case ARRAY:   return obj->data.v_array.size;
    }
    return -1;
}

bool snek_array_set(snek_object_t *snek_obj, size_t index, snek_object_t *value) {
    if (!snek_obj || !value || snek_obj->kind != ARRAY) {
        return false;
    }
    const int size = snek_obj->data.v_array.size;
    if ( index > size - 1  ) {
        return false;  // out of bounds
    }
    snek_obj->data.v_array.elements[index] = value;
    return true;
}

snek_object_t *snek_array_get(snek_object_t *snek_obj, size_t index) {
    if (!snek_obj ||  snek_obj->kind != ARRAY) {
        return false;
    }
    const int size = snek_obj->data.v_array.size;
    if ( index > size - 1  ) {
        return NULL;  // out of bounds
    }
    return snek_obj->data.v_array.elements[index];
}

snek_object_t *new_snek_array(size_t size) {
    snek_object_t *obj = malloc(sizeof(snek_object_t));
    if (!obj) {
        return NULL;
    }
    snek_object_t **elements = calloc(size, sizeof(void *));
    if (!elements) {
        free(obj);
        return NULL;
    }
    obj->kind = ARRAY;
    const snek_array_t array = {.size=size, .elements=elements};
    obj->data.v_array = array;

    return obj;
}


snek_object_t *new_snek_vector3(snek_object_t *x, snek_object_t *y, snek_object_t *z) {
    if ( !x || !y || !z) {
        return NULL;
    }

    snek_object_t *obj_ptr = malloc(sizeof(snek_object_t));
    if (!obj_ptr) {
        return NULL;
    }
    obj_ptr->kind = VECTOR3;
    const snek_vector_t v3 = { .x=x, .y=y, .z=z};

    snek_object_data_t data = {.v_vector3 = v3};
    obj_ptr->data = data;

    return obj_ptr;
}

snek_object_t *new_snek_string(char *value) {
    // printf("\nnew_snek_string: allocating %zu bytes for snek_object_t\n", sizeof(snek_object_t));
    // printf("enum snek_object_kind_t type size: %zu\n", sizeof(snek_object_kind_t));
    // printf("snek_object_data_t size: %zu\n", sizeof(snek_object_data_t));
    // printf("snek_object_t size; %zu\n\n", sizeof(snek_object_t));

    snek_object_t *obj_p = malloc(sizeof(snek_object_t));
    if (!obj_p) {
        return NULL;
    }

    size_t str_length = strlen(value);
    char *str_ptr = malloc(str_length + 1);
    if (!str_ptr) {
        free(obj_p);
        return NULL;
    }
    // printf("    allocating %zu bytes for string.\n", str_length + 1);

    strncpy(str_ptr, value, str_length);
    obj_p->kind = STRING;
    obj_p->data.v_string = str_ptr;
    return obj_p;
}

snek_object_t *new_snek_float(float value) {
    snek_object_t *sobj_ptr = malloc(sizeof(snek_object_t));
    if (!sobj_ptr) {
        return NULL;
    }
    sobj_ptr->kind = FLOAT;
    sobj_ptr->data.v_float = value;
    return sobj_ptr;
}

snek_object_t *new_snek_integer(int value) {
    snek_object_t *obj_p = malloc(sizeof(snek_object_t));
    if (!obj_p) {
        return NULL;
    }
    obj_p->kind = INTEGER;
    obj_p->data.v_int = value;
    return obj_p;
}


// --------------------------------------------------
// TEST CODE AND MAIN
// --------------------------------------------------

munit_case(RUN, test_str_copied, {
  char *input = "Hello, World!";
  snek_object_t *obj = new_snek_string(input);

  munit_assert_int(obj->kind, ==, STRING, "Must be a string!");

  // Should not have pointers be the same, otherwise we didn't copy the value.
  munit_assert_ptr_not_equal(obj->data.v_string, input,
                       "You need to copy the string.");

  // But should have the same data!
  //  This way the object can free it's own memory later.
  munit_assert_string_equal(obj->data.v_string, input,
                      "Should copy string correctly");

  // Should allocate memory for the string with null terminator.
  munit_assert_int_equal(boot_alloc_size(), 22, "Must allocate memory for string");

  // Free the string, and then free the object.
  free(obj->data.v_string);
  free(obj);
  assert(boot_all_freed());
});

int _main() {

    MunitTest tests[] = {
        munit_test("/copies_value", test_str_copied),
        munit_null_test,
    };

    MunitSuite suite = munit_suite("object-string", tests);

    return munit_suite_main(&suite, NULL, 0, NULL);
}
