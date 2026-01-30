//
// Created by Rob Ross on 1/27/26.
//

#include "range.h"


range range1(int stop) {
    const range r = {.start = 0, .stop = stop, .step = 1};
    return r;
}

range range2(int start, int stop) {
    const range r = {.start = start, .stop = stop, .step = 1};
    return r;
}

range range3(int start, int stop, int step) {
    const range r = {.start = start, .stop = stop, .step = step};
    return r;
}

