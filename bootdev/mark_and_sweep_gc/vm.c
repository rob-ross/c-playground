
#include "vm.h"
#include "../munit/memory_check.h"

void vm_frame_push(vm_t *vm, frame_t *frame) {
    if (!vm || !frame) {
        return;
    }
    stack_push(vm->frames, frame);
}

frame_t *vm_new_frame(vm_t *vm) {
    if (!vm) {
        return NULL;
    }
    frame_t *frame_p = malloc(sizeof(frame_t));
    if (!frame_p) {
        return NULL;
    }
    gc_stack_t *nsp = stack_new(8);
    if (!nsp) {
        free(frame_p);
        return NULL;
    }
    frame_p->references = nsp;
    vm_frame_push(vm, frame_p);
    return frame_p;
}

void frame_free(frame_t *frame) {
    if (!frame) {
        return;
    }
    stack_free(frame->references);
    free(frame);
}

vm_t *vm_new() {
    vm_t *vm_p = malloc(sizeof(vm_t));
    if (!vm_p) {
        return NULL;
    }
    gc_stack_t *frames_stack = stack_new(8);
    if (!frames_stack) {
        free(vm_p);
        return NULL;
    }
    gc_stack_t *objs_stack = stack_new(8);
    if (!objs_stack) {
        free(vm_p);
        free(frames_stack);
        return NULL;
    }
    vm_p->frames = frames_stack;
    vm_p->objects = objs_stack;

    return vm_p;
}

void vm_free(vm_t *vm) {
    if (!vm) {
        return;
    }
    if (vm->frames != NULL) {
        for (int i = 0; i < vm->frames->count; i++) {
            frame_free(vm->frames->data[i]);
        }
    }

    stack_free(vm->frames);
    stack_free(vm->objects);
    free(vm);
}

