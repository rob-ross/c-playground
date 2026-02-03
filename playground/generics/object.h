//
// Created by Rob Ross on 2/2/26.
//

#pragma once

typedef enum ObjectKind {
    OBJECT_KIND_START_OF_LIST = -1,
    OBJECT_KIND_BOOL,
    OBJECT_KIND_CHAR_P,
    OBJECT_KIND_CONST_CHAR_P,
    OBJECT_KIND_INTEGER,
    OBJECT_KIND_U_INTEGER,
    OBJECT_KIND_LONG,
    OBJECT_KIND_LONG_LONG,
    OBJECT_KIND_FLOAT,
    OBJECT_KIND_DOUBLE,

    OBJECT_KIND_UNKNOWN
} ObjectKind;


typedef struct type_info {
    ObjectKind kind;
    char * conversion_specifier;
    char * type_name;
    char * field_name;
} type_info;


typedef union PayloadData {
    _Bool   v_bool;
    char    *v_char_p;
    const char    *v_const_char_p;
    int     v_int;
    unsigned int v_uint;
    long    v_long;
    long long    v_long_long;
    float   v_float;
    double  v_double;
} PayloadData;

typedef struct ObjectPayload {
    // size_bytes is size of union PayloadData v-field in bytes. Only counts the scalar item in PayloadData.
    // Pointers are always 8 bytes, and this field does not include the size of the pointed-to memory.
    unsigned char size_bytes;
    PayloadData   data;

} ObjectPayload;

typedef struct Object {
    ObjectKind  kind;
    ObjectPayload payload;
} Object;

#define TYPE(expr, ...) _Generic( (expr), \
int:   OBJECT_KIND_INTEGER, \
float: OBJECT_KIND_FLOAT, \
double: OBJECT_KIND_DOUBLE, \
)

static inline void no_op(void) {

}

#define rob_new_object_char_p no_op
#define rob_new_object_const_char_p no_op

#define new_object( any_scalar ) \
_Generic( (any_scalar), \
_Bool:    rob_new_object(OBJECT_KIND_BOOL, (ObjectPayload){.size_bytes=sizeof(_Bool), .data=(PayloadData){.v_bool = any_scalar}}),\
int:    rob_new_object(OBJECT_KIND_INTEGER, (ObjectPayload){.size_bytes=sizeof(int), .data=(PayloadData){.v_int = any_scalar}}), \
unsigned int:    rob_new_object(OBJECT_KIND_U_INTEGER, (ObjectPayload){.size_bytes=sizeof(unsigned int), .data=(PayloadData){.v_uint = any_scalar}}), \
long:    rob_new_object(OBJECT_KIND_LONG, (ObjectPayload){.size_bytes=sizeof(long), .data=(PayloadData){.v_long = any_scalar}}), \
long long:    rob_new_object(OBJECT_KIND_LONG_LONG, (ObjectPayload){.size_bytes=sizeof(long long), .data=(PayloadData){.v_long_long = any_scalar}}), \
float:  rob_new_object(OBJECT_KIND_FLOAT,   (ObjectPayload){.size_bytes=sizeof(float), .data=(PayloadData){.v_float = any_scalar}}), \
double: rob_new_object(OBJECT_KIND_DOUBLE,   (ObjectPayload){.size_bytes=sizeof(double), .data=(PayloadData){.v_double = any_scalar}}) \
)


#define new_object_ptr( any_pointer ) \
_Generic( (any_pointer), \
 \
const char*:    rob_new_object(OBJECT_KIND_CONST_CHAR_P, (ObjectPayload){.size_bytes=sizeof(const char*), .data=(PayloadData){.v_const_char_p = any_pointer}}), \
char*:    rob_new_object(OBJECT_KIND_CHAR_P, (ObjectPayload){.size_bytes=sizeof(char*), .data=(PayloadData){.v_char_p = any_pointer}})\
)
