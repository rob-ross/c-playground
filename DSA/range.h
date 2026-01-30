//
// Created by Rob Ross on 1/27/26.
//

#ifndef RANGE_H
#define RANGE_H

typedef struct {
    const int start;
    const int stop;
    const int step;
} range;


range range1(int stop);
range range2(int start, int stop);
range range3(int start, int stop, int step);

// Macro “overload” dispatcher: new_range(x) / new_range(x,y) / new_range(x,y,z)
#define RANGE_GET_4TH_ARG(_1, _2, _3, NAME, ...) NAME
#define new_range(...) RANGE_GET_4TH_ARG(__VA_ARGS__, range3, range2, range1)(__VA_ARGS__)


#endif //RANGE_H