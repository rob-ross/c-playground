//
// Created by Rob Ross on 2/9/26.
//
// make:
// clang -std=c17 -o section_1.10.out section_1.10.c


#include <stdio.h>

#define MAXLINE 1000 // maximum input line size

int max;
char line[MAXLINE];     // current input line
char longest[MAXLINE];  // longest line saved here

int get_line(void);
void copy(void);

int main(void) {
    int len = 0;

    max = 0;
    while ( (len = get_line()) > 0 ) {
        if (len > max) {
            max = len;
            copy();
        }
    }

    if (max > 0) { // there was a line
        printf("\n\nmax line size = %d, line:\n%s", max, longest);
    }
    return 0;
}

// specialized version
int get_line(void) {
    int c = 0;
    int i = 0;
    for ( i = 0; i < MAXLINE - 1 && ( ( c = getchar() ) != EOF ) && c != '\n' ; i++) {
        line[i] = c;
    }
    if ( c == '\n' ) {
        line[i] = c;
        ++i;
    }
    line[i] = '\0';
    return i;
}

// specialized version
void copy(void) {
    int i = 0;
    // Ensure we don't overflow if logic changes later
    while (i < MAXLINE - 1 && (longest[i] = line[i]) != '\0') {
         ++i;
    }
}
