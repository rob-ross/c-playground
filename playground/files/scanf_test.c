#include <stdio.h>

int main() {
    int d_val, i_val;
    const char* input = "010 0x10";
    sscanf(input, "%d %i", &d_val, &i_val);
    printf("Input: %s\n", input);
    printf("% %d read 010 as: %d\n", d_val);
    printf("% %i read 0x10 as: %d\n", i_val);

    input = "010 010";
    sscanf(input, "%d %i", &d_val, &i_val);
    printf("Input: %s\n", input);
    printf("% %d read 010 as: %d\n", d_val);
    printf("% %i read 010 as: %d\n", i_val);

    return 0;
}
