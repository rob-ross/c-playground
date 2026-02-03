#include "vm.h"


#include "snekobject.h"
#include "stack.h"

#include "../../munit/memory_check.h"


void vm_collect_garbage(vm_t *vm) {
    mark(vm);
    trace(vm);
    sweep(vm);
}

void sweep(vm_t *vm) {
    if (!vm) {
        return;
    }
    for (size_t f = 0; f < vm->frames->count; f++) {
        frame_t *frame = vm->frames->data[f];
        for (size_t r = 0; r < frame->references->count; r++) {
            snek_object_t *obj = frame->references->data[r];
            if (obj->is_marked) {
                obj->is_marked = false;
                continue;
            }
            free(obj);
            obj = NULL;
        }
    }
    stack_remove_nulls(vm->objects);
}

void trace(vm_t *vm) {
    if (!vm) {
        return;
    }
    gc_stack_t *grayobjects = stack_new(8);
    if (!grayobjects) {
        return;
    }
    for (size_t f = 0; f < vm->frames->count; f++) {
        frame_t *frame = vm->frames->data[f];
        for (size_t r = 0; r < frame->references->count; r++) {
            snek_object_t *obj = frame->references->data[r];
            if (obj->is_marked) {
                stack_push(grayobjects, obj);
            }
        }
    }
    while (grayobjects->count > 0 ) {
        snek_object_t *obj = stack_pop(grayobjects);
        trace_blacken_object(grayobjects, obj);
    }
    stack_free(grayobjects);
}

void trace_blacken_object(gc_stack_t *gray_objects, snek_object_t *obj) {
    if (!gray_objects || ! obj) {
        return;
    }
    const snek_object_kind_t kind = obj->kind;
    if (kind == INTEGER || kind == FLOAT || kind == STRING) {
        //These don't contain references to other objects.
        return;
    }
    if (kind == VECTOR3) {
        trace_mark_object(gray_objects, obj->data.v_vector3.x);
        trace_mark_object(gray_objects, obj->data.v_vector3.y);
        trace_mark_object(gray_objects, obj->data.v_vector3.z);
    }
    if (kind == ARRAY) {
        for (size_t i = 0; i < obj->data.v_array.size; ++i) {
            trace_mark_object( gray_objects, snek_array_get(obj, i) );
        }
    }
}

void trace_mark_object(gc_stack_t *gray_objects, snek_object_t *obj) {
    if (!gray_objects || !obj || obj->is_marked) {
        return;
    }
    obj->is_marked = true;
    stack_push(gray_objects, obj);
}

void mark(vm_t *vm) {
    if (!vm) {
        return;
    }
    size_t num_frames = vm->frames->count;
    for (size_t f = 0; f < num_frames; ++f) {
        frame_t *frame = vm->frames->data[f];
        for (size_t r=0; r < frame->references->count; r++) {
            snek_object_t *sobj = frame->references->data[r];
            sobj->is_marked = true;
        }
    }
}

void frame_reference_object(frame_t *frame, snek_object_t *obj) {
    if (!frame || ! obj) {
        return;
    }
    stack_push(frame->references, obj);

}


void vm_free(vm_t *vm) {
    if (!vm) {
        return;
    }
    for (int i = 0; i < vm->frames->count; i++) {
        frame_free(vm->frames->data[i]);
    }
    for (int i = 0; i < vm->objects->count; i++) {
        snek_object_free(vm->objects->data[i]);
    }
    stack_free(vm->frames);
    stack_free(vm->objects);
    free(vm);
}

// don't touch below this line

vm_t *vm_new() {
    vm_t *vm = malloc(sizeof(vm_t));
    if (vm == NULL) {
        return NULL;
    }

    vm->frames = stack_new(8);
    vm->objects = stack_new(8);
    return vm;
}

void vm_track_object(vm_t *vm, snek_object_t *obj) {
    stack_push(vm->objects, obj);
}

void vm_frame_push(vm_t *vm, frame_t *frame) { stack_push(vm->frames, frame); }

frame_t *vm_frame_pop(vm_t *vm) { return stack_pop(vm->frames); }


frame_t *vm_new_frame(vm_t *vm) {
    frame_t *frame = malloc(sizeof(frame_t));
    frame->references = stack_new(8);

    vm_frame_push(vm, frame);
    return frame;
}

void frame_free(frame_t *frame) {
    stack_free(frame->references);
    free(frame);
}
