//
// Created by Rob Ross on 2/20/26.
//

#include <stdio.h>

enum corvid { magpie, raven, jay, chough, corvid_num, };

# define CORVID_NAMES /**/              \
    (char[corvid_num][8]){    \
        [chough] = "chough",            \
        [raven] = "raven",              \
        [magpie] = "magpie",            \
        [jay] = "jay",                  \
    }


// make :
// clang -std=c23 -o corvid_macro.out corvid_macro.c

int main(int argc, char *argv[]) {

    char const (* const bird)[8] = CORVID_NAMES;
    for (unsigned i = 0; i < corvid_num; ++i)
        printf("Corvid %u is the %s\n", i, bird[i]);
}
