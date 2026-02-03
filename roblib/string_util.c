//
// Created by Rob Ross on 2/1/26.
//

#include "string_util.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>




bool strings_equal(const char *str1, const char *str2) {
    const int len1 = strlen(str1);
    const int len2 = strlen(str2);
    if (len1 != len2) {
        return false;
    }
    for (int i = 0; i < len1; ++i) {
        if (str1[i] != str2[i]) {
            return false;
        }
    }
    return true;
}

bool strings_equal_case(const char *str1, const char *str2, Case c) {
    if (c == CASE_INSENSITIVE) {
        return strings_equal(str1, str2);
    }
    const int len1 = strlen(str1);
    const int len2 = strlen(str2);
    if (len1 != len2) {
        return false;
    }
    for (int i = 0; i < len1; ++i) {
        const char c1 = isalpha(str1[i]) ? toupper(str1[i]) : str1[i];
        const char c2 = isalpha(str2[i]) ? toupper(str2[i]) : str2[i];
        if ( c1 != c2 ) {
            return false;
        }
    }
    return true;
}

bool strings_same(const char *s1, const char *s2) {
    return s1 == s2;
}