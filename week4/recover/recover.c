#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// make: clang -std=c17 -o recover.out recover.c
struct jpg_signature {
    uint8_t first;
    uint8_t second;
    uint8_t third;
} jpg_signature = {.first=0xff, .second=0xd8, .third=0xff};

static const char *output_dir = "./out/";

void file_path(char *output_path, int image_number) {
    char file_path[512];
    char file_name[100];
    strcpy( file_path, output_dir);
    snprintf( file_name, 100-3, "%03i.jpg", image_number);
    strcat( file_path, file_name );

    strcpy(output_path, file_path);
}

bool is_jpg_header(const uint8_t *buffer) {
    if (memcmp(&jpg_signature, buffer, 3) == 0 && (buffer[3] & 0xE0) == 0xE0 ) {
        return true;
    }
    return false;
}

void t1(void) {

    FILE* input = fopen("card.raw", "r");
    if (!input) {
        perror("Error opening input file");
        exit(1);
    }

    // read file in 512 chunks. If the first 3 bytes match the signature look at the 4th byte and if it matches,
    // we start a new jpg. Keep reading 512b chunks until we see a signature at the start. This ends the last jpg
    // and starts a new jpg.
    int image_count = 0;
    const int buffer_size = 512;
    uint8_t buffer[buffer_size];

    FILE* output = NULL;
    char path_name[512];

    size_t read = 0;
    while ( (read = fread(buffer,  sizeof(uint8_t), buffer_size, input)) == buffer_size ) {
        if (is_jpg_header(buffer)) {
            if (output) {
                fclose(output);  // close currently opened output file.
            }
            file_path(path_name, image_count);
            output = fopen( path_name, "w");
            if (!output) {
                perror("Error opening output file");
                fclose(input);
                exit(1);
            }
            printf("New jpg file: %s\n", path_name);
            image_count++;
        }
        if (output) {
            fwrite(buffer, sizeof(uint8_t), buffer_size, output);
        }
    }

    if (read > 0 && read < buffer_size) {
        // last read was partial
        if (is_jpg_header(buffer)) {
            if (output) {
                fclose(output);  // close currently opened output file.
            }
            output = fopen( path_name, "w");
            if (!output) {
                perror("Error opening output file");
                fclose(input);
                exit(1);
            }
            printf("New jpg file: %s\n", path_name);
            image_count++;
        }
        if (output) {
            fwrite(buffer, sizeof(uint8_t), buffer_size, output);
        }

    }

    printf("image count: %i\n", image_count);



    fclose(input);
}

int main(int argc, char *argv[])
{
    t1();
}