//
// Created by Rob Ross on 2/10/26.
//

#include <limits.h>
#include <float.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <tgmath.h>

#include "../../roblib/string_utils.h"


enum type {
    CHAR, SCHAR, UCHAR,
    SHORT, USHORT,
    INT, UINT,
    LONG, ULONG,
    LONGLONG, ULONGLONG
};

const char * name_for_type(enum type type) {
    switch (type) {
        case CHAR: return "char";
        case SCHAR: return "signed char";
        case UCHAR: return "unsigned char";
        case SHORT: return "short";
        case USHORT: return "unsigned short";
        case INT: return "int";
        case UINT: return "unsigned int";
        case LONG: return "long";
        case ULONG: return "unsigned long";
        case LONGLONG: return "long long";
        case ULONGLONG: return "unsigned long long";
        default: return "unknown type";

    }
}

unsigned num_bytes_for(enum type type) {
    switch (type) {
        case CHAR:
        case SCHAR:
        case UCHAR: return sizeof(char);
        case SHORT:
        case USHORT: return sizeof(short);
        case INT:
        case UINT: return sizeof(int);
        case LONG:
        case ULONG: return sizeof(long);
        case LONGLONG:
        case ULONGLONG: return sizeof(long long);
        default: return 0;

    }
}

long long cast_value(enum type type, size_t value) {
    long long result = 0;
    switch (type) {
        case CHAR:
        case SCHAR:
        case UCHAR:  result = (unsigned char)value;  break;
        case SHORT:  result = (short)value;          break;
        case USHORT: result = (unsigned short)value; break;
        case INT:    result = (int)value;            break;
        case UINT:   result = (unsigned int)value;   break;
        case LONG:   result = (long)value;           break;
        case ULONG:  result = (unsigned long)value;  break;
        case LONGLONG:  result = (long long)value;   break;
        case ULONGLONG: result = (unsigned long long)value; break;
        default: result = 0; break;

    }

    return result;
}

struct Limits {
    long long min;
    long long max;
};

struct Limits compute_limits(enum type type) {
    const char *type_name = name_for_type(type);
    size_t num_bytes = num_bytes_for(type);
    size_t num_values= pow(2, num_bytes * 8);

    long long min = 0;
    long long max = 0;

    for (size_t value = 0; value < num_values; value++) {
        long long c_value = cast_value(type, value);

        // printf("(%zu, %lld), ", value, c_value);
        if (c_value < min) min = c_value;
        if (c_value > max) max = c_value;
    }

    // printf("\ntype: %s, num_bytes: %zu, num_values: %zu\n", type_name, num_bytes, num_values);

    return (struct Limits){.min = min, .max = max};
}

void display_limits(struct Limits computed_limits, const char *limit_name, struct Limits limits) {
    char min_label[100] = {0};
    char max_label[100] = {0};
    strcpy(min_label, limit_name);
    strcat(min_label, "_MIN");
    strcpy(max_label, limit_name);
    strcat(max_label, "_MAX");

    int label_len = strlen(min_label);


    printf(
        "%s: %lld, %s: %lld  (computed)\n",
        sutil_pad_right("min", label_len, ' '),  computed_limits.min,
        sutil_pad_right("max", label_len, ' '), computed_limits.max);

    printf("%s: %lld, %s: %lld  (limits.h)\n",
        sutil_pad_right(min_label, label_len, ' '),  limits.min,
        sutil_pad_right(max_label, label_len, ' '), limits.max);
}

__int128 pow_two_n(unsigned int n) {
    __int128 result = 1ULL;
    for (unsigned int i = 0; i < n; i++) {
        result = 2 * result;
    }
    return result;
}

// display an unsigned 128 bit integer
void print_u128(unsigned __int128 n) {
    unsigned long long high = (unsigned long long)(n >> 64);
    unsigned long long low = (unsigned long long)n;
    if (high > 0) {
        printf("%llu%019llu\n", high, low); // Simplified logic
    } else {
        printf("%llu\n", low);
    }
}

// display an signed 128 bit integer
void print_s128(__int128 n) {
    if (n == 0) {
        printf("0\n");
        return;
    }

    if (n < 0) {
        printf("-");
        // Handle negative numbers by converting to unsigned to avoid overflow
        // with the smallest possible negative value.
        unsigned __int128 un = (unsigned __int128)-(n + 1) + 1;
        char buf[45]; // __int128 max is ~3.4e38, so 45 chars is plenty
        int i = 0;
        while (un > 0) {
            buf[i++] = (char)((un % 10) + '0');
            un /= 10;
        }
        while (--i >= 0) putchar(buf[i]);
    } else {
        unsigned __int128 un = (unsigned __int128)n;
        char buf[45];
        int i = 0;
        while (un > 0) {
            buf[i++] = (char)((un % 10) + '0');
            un /= 10;
        }
        while (--i >= 0) putchar(buf[i]);
    }
    putchar('\n');
}

void display_long_limits(void) {
    unsigned num_bytes = 0;
    unsigned __int128 num_values = 0;
    __int128 min = 0;
    __int128 max = 0;

    // int
    num_bytes = num_bytes_for(INT);
    num_values = pow_two_n(num_bytes * 8);
    min =  (__int128)-(num_values / 2) ;
    max =  (__int128)((num_values / 2) - 1);
    printf("\nint:\n");
    printf("type: %s, num_bytes: %u, num_values: %llu\n", "int", num_bytes, (unsigned long long)num_values);
    // signed int
    printf("\nsigned int:\n");
    printf( "%s: %lld, %s: %lld  (computed)\n", "min", (long long)min, "max", (long long)max);
    printf( "%s: %d, %s: %d  (limits.h)\n", "min", INT_MIN, "max", INT_MAX);

    // unsigned int
    max =  (__int128)(num_values  - 1);
    printf("\nusigned int:\n");
    printf( "%s: %d, %s: %u  (computed)\n", "min", 0, "max", (unsigned int)max);
    printf( "%s: %d, %s: %u  (limits.h)\n", "min", 0, "max", UINT_MAX);



    // long
    num_bytes = num_bytes_for(LONG);
    num_values = pow_two_n(num_bytes * 8);
    min =  (__int128)-(num_values / 2) ;
    max =  (__int128)((num_values / 2) - 1);
    printf("\nlong:\n");
    printf("type: %s, num_bytes: %u, num_values: %llu + 1\n", "long", num_bytes, (unsigned long long)(num_values - 1));

    //signed long:
    printf("\nsigned long:\n");
    printf( "%s: %lld, %s: %lld  (computed)\n", "min", (long long)min, "max", (long long)max);
    printf( "%s: %ld, %s: %ld  (limits.h)\n", "min", LONG_MIN, "max", LONG_MAX);

    // unsigned long
    max =  (__int128)(num_values - 1);
    printf("\nunsigned long\n");
    printf( "%s: %d, %s: %lu  (computed)\n", "min", 0, "max", (unsigned long)max);
    printf( "%s: %llu, %s: %lu  (limits.h)\n", "min", 0LL, "max", ULONG_MAX);



    // long long
    num_bytes = num_bytes_for(LONGLONG);
    num_values = pow_two_n(num_bytes * 8);
    min =  (__int128)-(num_values / 2) ;
    max =  (__int128)((num_values / 2) - 1);

    printf("\nlong long:\n");
    printf("type: %s, num_bytes: %u, num_values: %llu + 1\n", "long", num_bytes, (unsigned long long)(num_values -1));

    // signed long long:
    printf("\nsigned long long:\n");
    printf( "%s: %lld, %s: %lld  (computed)\n", "min", (long long)min, "max", (long long)max);
    printf( "%s: %lld, %s: %lld  (limits.h)\n", "min", LLONG_MIN, "max", LLONG_MAX);


    // unsigned long long
    max = (unsigned long long)(num_values - 1);
    printf("\nunsiged long long:\n");
    printf( "%s: %llu, %s: %llu  (computed)\n", "min", 0LL, "max", (unsigned long long)max);
    printf( "%s: %llu, %s: %llu  (limits.h)\n", "min", 0LL, "max", ULLONG_MAX);

}


void all_limits(void) {
    struct Limits limits = {0};
    // char ranges
    printf("\nchar:\n");
    limits = compute_limits(CHAR);
    display_limits(limits, "CHAR", (struct Limits){.min = CHAR_MIN, .max = CHAR_MAX});
    printf("\nsigned char:\n");
    limits = compute_limits(SCHAR);
    display_limits(limits, "SCHAR", (struct Limits){.min = SCHAR_MIN, .max = SCHAR_MAX});
    printf("\nunsigned char:\n");
    limits = compute_limits(UCHAR);
    display_limits(limits, "UCHAR", (struct Limits){.min = 0, .max = UCHAR_MAX});

    // short ranges
    printf("\nshort:\n");
    limits = compute_limits(SHORT);
    display_limits(limits, "SHRT", (struct Limits){.min = SHRT_MIN, .max = SHRT_MAX});
    printf("\nunsigned short:\n");
    limits = compute_limits(USHORT);
    display_limits(limits, "USHRT", (struct Limits){.min = 0, .max = USHRT_MAX});

    // int ranges
    // printf("\nint:\n");
    // limits = compute_limits(INT);
    // display_limits(limits, "INT", (struct Limits){.min = INT_MIN, .max = INT_MAX});
    // printf("\nunsigned int:\n");
    // limits = compute_limits(UINT);
    // display_limits(limits, "UINT", (struct Limits){.min = 0, .max = UINT_MAX});

    // long would take too long to compute by iterating over all longs (~1300 YEARS) . So we'll just calculate them.
    display_long_limits();


}

void float_limits(void) {
    printf("\nFloat Limits\n");
    printf("-----------------\n");

    // float
    printf("\nfloat (size: %zu) limits:\n", sizeof(float));
    printf("mantissa digits (in bits): %d, digits: %d, min exp: %d, min10 exp: %d, max exp: %d, max10 exp: %d, epsilon: %.25f, min: %.50f, true min: %.50f, decimal dig: %d, has subnorm: %d\n", FLT_MANT_DIG, FLT_DIG, FLT_MIN_EXP, FLT_MIN_10_EXP, FLT_MAX_EXP, FLT_MAX_10_EXP, FLT_EPSILON, FLT_MIN, FLT_TRUE_MIN, FLT_DECIMAL_DIG, FLT_HAS_SUBNORM);
    printf("max: %f\n", FLT_MAX);

    // 340282346638528859811704183484516925440.000000

    // double
    printf("\ndouble (size: %zu) limits:\n", sizeof(double));
    printf("mantissa digits (in bits): %d, digits: %d, min exp: %d, min10 exp: %d, max exp: %d, max10 exp: %d, epsilon: %f, min: %f, true min: %f, decimal dig: %d, has subnorm: %d\n", DBL_MANT_DIG, DBL_DIG, DBL_MIN_EXP, DBL_MIN_10_EXP, DBL_MAX_EXP, DBL_MAX_10_EXP, DBL_EPSILON, DBL_MIN, DBL_TRUE_MIN, DBL_DECIMAL_DIG, DBL_HAS_SUBNORM);
    printf("max: %f\n", DBL_MAX);

    //max: 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.000000


    // long double
    printf("\nlong double (size: %zu) limits:\n", sizeof(long double));
    printf("mantissa digits (in bits): %d, digits: %d, min exp: %d, min10 exp: %d, max exp: %d, max10 exp: %d, epsilon: %Lf, min: %Lf, true min: %Lf, decimal dig: %d, has subnorm: %d\n", LDBL_MANT_DIG, LDBL_DIG, LDBL_MIN_EXP, LDBL_MIN_10_EXP, LDBL_MAX_EXP, LDBL_MAX_10_EXP,  LDBL_EPSILON, LDBL_MIN, LDBL_TRUE_MIN, LDBL_DECIMAL_DIG, LDBL_HAS_SUBNORM);
    printf("max: %Lf\n", LDBL_MAX);

    /**
    *FLT_MANT_DIG __FLT_MANT_DIG__
#define DBL_MANT_DIG __DBL_MANT_DIG__
#define LDBL_MANT_DIG __LDBL_MANT_DIG__
     */
}




void calc_limits(void) {
    //maximum finite value / most positive value
    int float_bits = 32;
    int double_bits = 64;
    int ldouble_bits = 128;
    int mac_ldouble_bits = 80; // standard is 128 but Mac os uses 80

    int float_exp_bits = 8;
    int double_exp_bits = 11;
    int ldouble_exp_bits = 15;

    int float_max_exp = pow(2, float_exp_bits - 1) - 1;
    int double_max_exp = pow(2, double_exp_bits - 1) - 1;
    int ldouble_max_exp = pow(2, ldouble_exp_bits - 1) - 1;

    // 1 bit for sign
    int float_mantissa_bits = float_bits - 1 - float_exp_bits;
    int double_mantissa_bits = double_bits - 1 - double_exp_bits;
    int ldouble_mantissa_bits = ldouble_bits - 1 - ldouble_exp_bits;
    int mac_ldouble_mantissaa_bits = mac_ldouble_bits - 1 - ldouble_exp_bits;

    double  float_max_positive = (2 - pow(2, -float_mantissa_bits)) * pow(2, float_max_exp);

    long double double_max_positive = 2 * pow(2, double_max_exp);

    printf("calc limits\n");

    printf("Float:\n");
    printf("bits: %d, exp bits: %d, mantissa bits: %d, max exp: %d, MPV: %e\n", float_bits, float_exp_bits, float_mantissa_bits, float_max_exp,  float_max_positive);

    printf("Double:\n");
    printf("bits: %d, exp bits: %d, mantissa bits: %d, max exp: %d, MPV: %Le\n", double_bits, double_exp_bits, double_mantissa_bits, double_max_exp,  double_max_positive);

}

// make:
// clang -std=c17 -o data_ranges.out data_ranges.c ../../roblib/string_utils.c

int main(void) {
    all_limits();
    float_limits();
    printf("\n");

    calc_limits();

}
