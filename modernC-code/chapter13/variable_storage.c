// variable_storage.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/19 16:06:55 PDT

//
// Created by Rob Ross on 3/19/26.
//

int a = 22;  // initialized, static storage *duration* (distinct from storage *class*)

extern int E = 5; // extern initialized, static storage duration

// string literal, in file or block scope, has static storage duration
// initialized static variable has static storage durations (also internal linkage)
static char *foo = "foo";

constexpr float PI = 3.14159;

void t1(void) {
    char const A[] = { 'e', 'n', 'd', '\0', };
    char const B[] = { 'e', 'n', 'd', '\0', };
    char const* c = "end";
    char const* d = "end";
    char const* e = "friend";
    char const* f = (char const[]){ 'e', 'n', 'd', '\0', };
    char const* g = (char const[]){ 'e', 'n', 'd', '\0', };

    // even though "boo" is a string literal we can assign it to a non-const qualified char[].
    // but string literals are in the data segment and shouldn't be writeable by our code.
    char *h = "boo";
    //h[0] = 'f'; // should cause an error signal 10:SIGBUS. And it does!
    char const *const_h = "boo";
    //const_h[0] = 'f'; // compiler doesn't let us even try with const qualifier.

    /*if (A ==  B) {
        // shamalamma. You can't compare Arrays, silly. They don't have values.
    }*/
}

int main(int argc, char *argv[]) {
    t1();
}
