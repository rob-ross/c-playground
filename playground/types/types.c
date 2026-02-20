//
// Created by Rob Ross on 1/6/26.
//

#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <stddef.h> // for ptrdiff_t
#include <assert.h> // for assert()

// C11 _Generic selection to mimic a typeof() string for inspection
#define typename(x) _Generic((x), \
    _Bool: "_Bool", \
    unsigned char: "unsigned char", \
    char: "char", \
    signed char: "signed char", \
    short int: "short int", \
    unsigned short int: "unsigned short int", \
    int: "int", \
    unsigned int: "unsigned int", \
    long int: "long int", \
    unsigned long int: "unsigned long int", \
    long long int: "long long int", \
    unsigned long long int: "unsigned long long int", \
    float: "float", \
    double: "double", \
    long double: "long double", \
    char *: "pointer to char", \
    void *: "pointer to void", \
    int *: "pointer to int", \
    default: "other")

void std_int_types(void) {
    typedef int8_t byte;

    //regular signed ints
    // ReSharper disable CppLocalVariableMayBeConst
    byte b = 127;
    short int si = -10;
    int i = -1;
    long int li = 100;
    long long int lli = 1000;

    printf("byte b = %hhd\n", b);
    printf("short int si = %hd\n", si);
    printf("int i = %d\n", i);
    printf("long int li = %ld\n", li);
    printf("long long int lli = %lld\n", lli);

    //unsigned ints, "unsigned", "signed" are 'qualifiers')
    typedef uint8_t ubyte;
    ubyte ub = 255;
    unsigned short int usi = 10;
    unsigned int ui = 1;
    unsigned long int uli = 100;
    unsigned long long int ulli = 1000;

    printf("\n");
    printf("unsigned ubyte ub = %hhu\n", ub);
    printf("unsigned short int usi = %hu\n", usi);
    printf("unsigned int ui = %u\n", ui);
    printf("unsigned long int uli = %lu\n", uli);
    printf("unsigned long long int ulli = %llu\n", ulli);

    byte sb = -127;
    printf("\nsigned byte -127 as unsigned byte= %hhu\n", sb);
    printf("unsigned ubyte 255 as signed byte = %hhd\n", ub);


}

void float_types(void) {
    float  f = 42.42;
    double d = 42.42;
    long double ld = 42.42;  // may have same length as double.

    printf("\nsizeof float: %lu, sizeof double: %lu, sizeof long double: %lu\n", sizeof(f), sizeof(d), sizeof(ld));
}

void char_types(void) {
    // Single byte (Legacy Extended ASCII) - often fails in modern terminals
    unsigned char legacy = 163;
    printf("\nLegacy Byte: %c (Value: %hhu)\n", legacy, legacy);

    // UTF-8 String (Modern way) - works in CLion's default settings
    // The £ symbol is actually TWO bytes in UTF-8: 0xC2 0xA3
    char *modern = "£";
    printf("Modern UTF-8: %s\n", modern);

    char c = 'c';
    signed char sc = -128;
    unsigned char uc = 169;

    printf("char c = %c\n", c);
    printf("signed char c = %c\n", sc);
    printf("unsigned char as int byte uc = %hhu\n", uc);
    printf("unsigned char as char uc = %c\n", uc);

}

void conversion_check(void) {
    // 5u is unsigned int, -2L is signed long int.
    // On systems where LONG_MAX >= UINT_MAX (like 64-bit macOS/Linux), this promotes to long int.
    // On systems where long is same size as int (like Windows), this promotes to unsigned long int.
    printf("\n--- Implicit Conversion Check ---\n");
    printf("Type of (5u * -2L): %s\n", typename(5u * -2L));

    // C23 typeof() demonstration
    // We declare 'y' based on the expression's type, then use typename() to see what it became.
    typeof(5u * -2L) y = 5u * -2L;
    printf("Type of variable 'y' declared via typeof: %s\n", typename(y));
    printf("Value of y: %ld\n", (long)y);

    // Demonstration of the "Trap" regarding signed/unsigned promotion
    printf("\n--- The 'Same Size' Trap ---\n");
    printf("sizeof(int) = %zu, sizeof(long) = %zu\n", sizeof(int), sizeof(long));

    if ((5u * -2L) < 0) {
        printf("Logic check: (5u * -2L) < 0 is TRUE. Result is signed (Safe).\n");
    } else {
        printf("Logic check: (5u * -2L) < 0 is FALSE. Result is unsigned (Dangerous)!\n");
        printf("Actual unsigned value: %lu\n", (unsigned long)(5u * -2L));
    }

    if ((5uL * -2L) < 0) {
        printf("Logic check: (5uL * -2L) < 0 is TRUE. Result is signed (Safe).\n");
    } else {
        printf("\n--- The 'Unsigned Wins' Trap ---\n");
        printf("Logic check: (5uL * -2L) < 0 is FALSE.\n");
        printf("Reason: -2L converted to huge unsigned number before multiply.\n");
        printf("Result (unsigned): %lu\n", (5uL * -2L));
    }

    printf("\n--- Bit Pattern Verification ---\n");
    
    // Case 1: Same Size (long -> unsigned long)
    // The bits are preserved exactly.
    long l_neg = -1L;
    unsigned long ul_same = (unsigned long)l_neg;
    printf("1. Same Size (long -> ulong): Bits are IDENTICAL.\n");
    printf("   Signed -1L (hex):   %016lX\n", (unsigned long)l_neg);
    printf("   Unsigned cast (hex):%016lX\n", ul_same);

    // Case 2: Widening (int -> unsigned long long)
    // The bits CHANGE because C sign-extends the negative number to fill the new space.
    int i_neg = -1;
    unsigned long long ull_wide = (unsigned long long)i_neg;
    printf("2. Widening (int -> ulonglong): Bits CHANGE (Sign Extension).\n");
    printf("   Signed -1 (hex):    %08X\n", (unsigned int)i_neg);
    printf("   Unsigned cast (hex):%016llX (Note the extra Fs)\n", ull_wide);
}

void overflow_check(void) {
    printf("\n--- Overflow/Wrapping Check ---\n");

    // Case 1: Signed Integer Overflow (Undefined Behavior)
    // We use volatile to force runtime evaluation and prevent compiler optimization.
    volatile int a = INT_MAX;
    volatile int b = INT_MAX;
    
    // NOTE: In C, signed overflow is Undefined Behavior. 
    // On most 2's complement machines, this wraps to 1, but don't rely on it!
    int signed_res = a * b; 
    printf("1. Signed int overflow (INT_MAX * INT_MAX):\n");
    printf("   Type: %s\n", typename(a * b));
    printf("   Result: %d (Undefined Behavior, usually wraps)\n", signed_res);

    // Case 2: Unsigned Integer Wrapping (Defined Behavior)
    volatile unsigned int ua = UINT_MAX;
    volatile unsigned int ub = UINT_MAX;
    unsigned int unsigned_res = ua * ub;
    printf("2. Unsigned int wrapping (UINT_MAX * UINT_MAX):\n");
    printf("   Type: %s\n", typename(ua * ub));
    printf("   Result: %u (Guaranteed Modulo Arithmetic)\n", unsigned_res);

    printf("\n--- The Assignment Trap ---\n");
    volatile int x = 2000000000; // 2 billion
    volatile int y = 2;          // Result 4 billion overflows 32-bit signed int

    // INCORRECT: The multiplication happens as 'int' BEFORE the assignment to 'long'
    long bad_product = x * y;
    printf("Incorrect (long p = x * y): %ld (Overflow occurred before assignment)\n", bad_product);

    // CORRECT: Cast at least one operand to 'long' to force the math to happen in 'long'
    if (sizeof(long) > sizeof(int)) {
        long good_product = (long)x * y;
        printf("Correct (long p = (long)x * y): %ld\n", good_product);
    } else {
        printf("Skipping correct demo: sizeof(long) == sizeof(int) on this system.\n");
    }
}

void mixed_check(void) {
    printf("\n--- Mixed Expression Check (UINT_MAX * -2L) ---\n");
    printf("sizeof(int) = %zu, sizeof(long) = %zu\n", sizeof(int), sizeof(long));

    // Expression: UINT_MAX * -2L
    // 1. UINT_MAX is unsigned int.
    // 2. -2L is signed long.
    
    printf("Type of (UINT_MAX * -2L): %s\n", typename(UINT_MAX * -2L));

    if (sizeof(long) > sizeof(int)) {
        printf("Case: long > int. Promotes to SIGNED long. No overflow.\n");
        printf("Value: %ld\n", (long)(UINT_MAX * -2L));
    } else {
        printf("Case: long == int. Promotes to UNSIGNED long. Wraps to 2.\n");
        printf("Value: %lu\n", (unsigned long)(UINT_MAX * -2L));
    }

    printf("Type of (ULONG_MAX * -2L): %s\n", typename(ULONG_MAX * -2L));

    printf("Case: long == long. Promotes to UNSIGNED long. Wraps to 2.\n");
    printf("Value: %lu\n", (unsigned long)(ULONG_MAX * -2L));
}

void unsigned_trap_check(void) {
    printf("\n--- Why Prefer Signed Types? (The Underflow Trap) ---\n");

    // Scenario: Calculating distance or difference between two values
    unsigned int u_a = 5;
    unsigned int u_b = 10;

    // DANGER: Unsigned subtraction wraps around!
    // You expect -5, but you get UINT_MAX - 4
    unsigned int u_diff = u_a - u_b;
    printf("Unsigned: 5u - 10u = %u (Logic Error: Result looks huge)\n", u_diff);

    // SAFE: Signed subtraction
    int s_a = 5;
    int s_b = 10;
    int s_diff = s_a - s_b;
    printf("Signed:   5 - 10   = %d (Correct: Negative value)\n", s_diff);
}

void size_t_check(void) {
    printf("\n--- The size_t Trap ---\n");

    // Scenario 1: The "Pollution"
    // You have a size_t (from strlen or sizeof) and you mix it with a negative int.
    size_t len = 10;
    int delta = -5;

    printf("Expression: (size_t)10 + (int)-5\n");
    if (len + delta < 10) {
        printf("Result < 10 (Math is intuitive).\n");
    } else {
        printf("Result > 10 (Math is BROKEN).\n");
        printf("Reason: int promotes to size_t (unsigned). -5 becomes huge.\n");
        printf("Value: %zu\n", len + delta);
    }

    // Scenario 2: Reverse Loops
    // for (size_t i = len - 1; i >= 0; i--)  <-- INFINITE LOOP
    printf("\nReverse Loop Fixes:\n");
    
    // Fix A: The "Arrow" operator (k goes to 0)
    // This is a common C idiom for unsigned reverse loops.
    printf("A) while (k --> 0):\n");
    size_t k = 3;
    while (k --> 0) {
        printf("   k = %zu\n", k);
    }

    // Fix B: Cast to signed (ptrdiff_t is the signed equivalent of size_t)
    printf("B) Cast to ptrdiff_t (or long/int):\n");
    for (ptrdiff_t i = (ptrdiff_t)len - 1; i >= 0; i -= 5) { 
        printf("   i = %td\n", i);
    }
}

void static_param_check(int data[static 3]) {
    printf("\n--- Static Array Parameter Check (C99/C23) ---\n");
    printf("Function declared as: void static_param_check(int data[static 3])\n");
    
    // LINGUISTIC HELP:
    // Read 'static 3' as: "The array has a STATIC (guaranteed/fixed) minimum size of 3."
    // It is a promise that the size 3 is invariant.
    
    // 1. It decays to a pointer, just like normal array parameters
    printf("Type of 'data' inside function: %s\n", typename(data));
    
    // 2. The 'static 3' is a contract:
    //    - OPTIMIZATION: The compiler assumes 'data' is NOT NULL and removes internal null checks.
    //      (NOTE: Do not write 'if (!data) ...' here. The compiler is allowed to delete that check!)
    
    //    - DEBUGGING: We can use assert() to verify the contract during development.
    //      If compiled with -DNDEBUG (Release mode), this line is completely removed by the preprocessor.
    //      This grants your "Java Wish": zero cost in production, safety in debug.
    assert(data != NULL && "Violation of [static 3] contract: data is NULL");

    //    - EFFICIENCY: Callers passing valid stack addresses (e.g. &x) pay zero performance cost.
    //    - SAFETY: Static analysis tools (not the runtime) will flag if you pass NULL here.
    //    - WARNING: Passing NULL at runtime is Undefined Behavior (crash/exploit risk).
    
    // 3. The Size Guarantee:
    //    The compiler assumes data[0], data[1], and data[2] are valid memory.
    printf("Accessing data[2] (guaranteed to exist): %d\n", data[2]);
}

void optional_pointer_check(int *data) {
    printf("\n--- Optional Pointer Check (Standard C) ---\n");
    // Here, NULL is a VALID state, not an error.
    // We MUST check for it with 'if', because 'assert' would crash valid code.
    if (data == NULL) {
        printf("Data is NULL. Using default behavior (no crash).\n");
    } else {
        printf("Data is valid. Value: %d\n", *data);
    }
}

void restrict_check(int n, int a[restrict], int b[restrict]) {
    printf("\n--- Restrict Qualifier Check ---\n");
    printf("Function declared as: void restrict_check(int n, int a[restrict], int b[restrict])\n");
    
    // LINGUISTIC HELP:
    // Read 'restrict' as: "This pointer (and values derived from it) is the ONLY way to access this memory."
    // It is a promise of EXCLUSIVITY (No Aliasing).
    
    // 1. The Problem (Aliasing):
    //    Without restrict, the compiler fears 'a' and 'b' might overlap.
    //    If a[0] is the same memory as b[0], writing to a[0] changes b[0].
    //    This forces the compiler to reload b[0] from memory every time, killing performance.
    
    // 2. The Solution (Restrict):
    //    We promise they are distinct. The compiler can now:
    //    - Load 'b' into registers once.
    //    - Use SIMD (Single Instruction Multiple Data) instructions to process multiple ints at once.
    
    // 3. The DANGER (Violating the Contract)
    //    A naive caller CAN pass overlapping pointers. This is Undefined Behavior.
    //    We use assert() to catch this during development. This check verifies that the
    //    memory ranges [a, a+n) and [b, b+n) do not overlap.
    //    In release builds (-DNDEBUG), this check disappears, leaving only the performance gain.
    assert(a + n <= b || b + n <= a && "Violation of restrict contract: memory regions overlap!");
    
    // 3. Local Aliases:
    //    It is perfectly legal to create a local pointer FROM a restricted pointer.
    //    The compiler knows 'cursor' comes from 'a', so the contract is kept.
    int *cursor = a;
    for (int i = 0; i < n; i++) {
        *cursor += b[i];
        cursor++;
    }
    printf("Computed vectorizable sum. Result a[0]: %d\n", a[0]);
}

int main (void) {
    std_int_types();
    char_types();
    float_types();
    conversion_check();
    overflow_check();
    mixed_check();
    unsigned_trap_check();
    size_t_check();

    // Demo for [static N]
    int my_array[] = {10, 20, 30, 40};
    static_param_check(my_array);

    // Demo for Optional Pointer
    optional_pointer_check(NULL);      // Valid usage
    optional_pointer_check(my_array);  // Valid usage

    // Demo for restrict
    int x_arr[] = {1, 2, 3};
    int y_arr[] = {4, 5, 6};
    restrict_check(3, x_arr, y_arr);

    // Uncommenting the line below would (and should) crash in a debug build.
    // restrict_check(3, x_arr, x_arr); // VIOLATION: a and b are the same
    int32_t a;
}