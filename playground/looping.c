//
// Created by Rob Ross on 2/12/26.
//

#include <limits.h>
#include <stdio.h>

void loop_ints(void) {
    // this loop takes about 8-10 seconds on my MacBook Pro
    printf("About to loop over all ints, INT_MIN to INT_MAX\n");
    int x;
    for (int i = INT_MIN; i < INT_MAX; i++) {
        int y;
        y = i + x;
    }
    printf("loop done!\n");

}


int main(void) {
    loop_ints();
}