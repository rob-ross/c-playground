//
// Created by Rob Ross on 2/3/26.
//

// file pointers are FILE*.


#include <stdio.h>
// assumes file_name is a text file and not too large.
// reads and prints one char at a time via fgetc and putchar.
void display_file_fgetc(const char *file_name) {
    if (!file_name) {
        return;
    }

    //is there a method to tell you if the file exists? Or is a NULL return from fopen() the only clue?
    FILE* file = fopen(file_name, "r");
    if (!file) {
        printf("FILE* NULL after: fopen('%s')\n", file_name);
    }

    // read and print one character at a time:
    printf("Displaying contents of file: %s\n", file_name);
    printf("---------------------------------------------\n");
    int c = 0;
    while ( (c = fgetc(file)) != EOF) {
        putchar(c);
    }

    fclose(file);

}
// reads and prints a buffer at a time via fread
void display_file_fread(const char *file_name) {
    if (!file_name) {
        return;
    }
    FILE* file = fopen(file_name, "r");
    if (!file) {
        printf("FILE* NULL after: fopen('%s')\n", file_name);
    }
    printf("Displaying contents of file: %s\n", file_name);
    printf("---------------------------------------------\n");

#define ROB_BUFFER_SIZE_ 1024
    int buffer[ROB_BUFFER_SIZE_];

    size_t read = 0L;

    while (  (read = fread(buffer, sizeof(int), ROB_BUFFER_SIZE_ - 1, file)) ==  ROB_BUFFER_SIZE_ - 1) {
        size_t chars_read = sizeof(int) * read;
        buffer[ read ] = 0;
        // printf("\nbuffer read %zu elements, %zu characters:\n%s", read, chars_read, (char*)buffer);
        printf("%s", (char*)buffer);
    }
    // partial buffer
    buffer[ read ] = 0;
    printf("%s\n", (char*)buffer);
#undef ROB_BUFFER_SIZE_

    printf("EOF reached: %i, error: %i, perror: ", feof(file), ferror(file));

    perror("Sample Error Msg"); //print error message to stderror

    fclose(file);


}

int main(void) {
    display_file_fread("pointers.c");
}