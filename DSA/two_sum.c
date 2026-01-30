//
// Created by Rob Ross on 1/27/26.
//
//

//to build: clang two_sum.c str_utils.c range.c -o two_sum

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "str_utils.h"

/*
 * For the given array of int and a target_sum, return true if there are two elements in the array
 * that sum to target_sum. Assumes nums is sorted in ascending order
*/
bool two_sum_decision(const int *nums, const size_t nums_size, const int target_sum) {
    if (nums == NULL || nums_size <= 2) {
        return false;
    }

    size_t left_index = 0;
    size_t right_index = nums_size - 1;
    while (left_index < right_index ) {
        const int sum = nums[left_index] + nums[right_index];
        if (  sum == target_sum ) {
            return true;
        }
        if ( sum < target_sum ) {
            left_index++;
        } else {
            right_index--;
        }
    }
    return false;
}



void t_two_sum_decision(void) {
    const int nums[] = { 1, 2, 3, 4, 5 };
    const int size_nums = 5;
    int target_sum = 5;

    char *array_str = astr(nums, size_nums);

    bool result = 0;

    result = two_sum_decision(nums, size_nums, target_sum);
    printf("two_sum_decision(nums=%s, 5, 5) = %s = %i\n", array_str, bstr(result), result);

    target_sum = 1;
    result = two_sum_decision(nums, size_nums, target_sum);
    printf("two_sum_decision(nums=%s, 5, 1) = %s = %i\n", array_str, bstr(result), result);

    target_sum = 11;
    result = two_sum_decision(nums, size_nums, target_sum);
    printf("two_sum_decision(nums=%s, 5, 11)= %s = %i\n", array_str, bstr(result), result);


    free(array_str);
}




int main(void) {
    main_entry();

    // t_two_sum_decision();

    // if reporting failure, `return EXII_FAILURE` else `return EXIT_SUCCESS`
}
