//
// Created by Rob Ross on 2/2/26.
//

#include "../object.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


Object * rob_new_object(const ObjectKind kind, const ObjectPayload payload) {
    if (  !(kind > OBJECT_KIND_START_OF_LIST && kind < OBJECT_KIND_UNKNOWN)) { return NULL; }

    Object *obj = malloc(sizeof(Object));
    if (!obj) {
        return NULL;
    }
    obj->kind = kind;
    obj->payload = payload;
    return obj;

}

void display_object(Object *obj) {
    if (!obj) {
        printf("Object(NULL)\n");
        return;
    }
    printf("Object(kind=%i, size=%i, ", obj->kind, obj->payload.size_bytes);
    switch (obj->kind) {
        case OBJECT_KIND_BOOL:
            printf("v_bool=%i)\n", obj->payload.data.v_bool);
            break;
        case OBJECT_KIND_CHAR_P:
            printf("v_char_p=%s)\n", obj->payload.data.v_char_p);
            break;
        case OBJECT_KIND_CONST_CHAR_P:
            printf("v_const_char_p=%s)\n", obj->payload.data.v_const_char_p);
            break;
        case OBJECT_KIND_INTEGER:
            printf("v_int=%i)\n", obj->payload.data.v_int);
            break;
        case OBJECT_KIND_U_INTEGER:
            printf("v_uint=%u)\n", obj->payload.data.v_uint);
            break;
        case OBJECT_KIND_LONG:
            printf("v_long=%li)\n", obj->payload.data.v_long);
            break;
        case OBJECT_KIND_LONG_LONG:
            printf("v_long_long=%lli)\n", obj->payload.data.v_long_long);
            break;
        case OBJECT_KIND_FLOAT:
            printf("v_float=%f)\n", obj->payload.data.v_float);
            break;
        case OBJECT_KIND_DOUBLE:
            printf("v_double=%f)\n", obj->payload.data.v_double);
            break;
        default:
            printf("Object(unknown kind)\n");
    }
}



int main(void) {
    // we'd like to write something like
    // int foo = 42;
    // Object(42); // and have this create an Object of type int. This means:
    // kind = OBJECT_KIND_INTEGER,
    // payload.size_bytes == 4,
    // payload.data.v_int == 42

    int foo = 42;
    Object *i = new_object( foo);
    display_object(i);
    float bar = 1.0f;


    Object *f = new_object( bar );

    display_object(f);

    display_object(new_object(43));
    display_object(new_object(1.0));

    unsigned int baz = 44;
    display_object(new_object(baz));

    display_object(new_object(42L));
    display_object(new_object(42LL));

    display_object(new_object_ptr("This is text"));

    const char *cs = "this is const char* text";
    char * s = "this is char * text";

    display_object(new_object_ptr(cs));
    display_object(new_object_ptr(s));

}