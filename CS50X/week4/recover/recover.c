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
const int BUFFER_SIZE = 512;

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

bool start_new_jpg_file(FILE** output, const int image_count) {
    if (*output) {
        fclose(*output);  // close currently opened output file.
    }
    char path_name[512];
    file_path(path_name, image_count);
    FILE* o = fopen( path_name, "w");
    if ( !o ) {
        fprintf(stderr, "Could not create output file: %s\n", path_name);
        perror("Error");
        return false;
    }
    // printf("New jpg file: %s\n", path_name);
    *output = o;
    return true;
}

bool process_buffer(const uint8_t *buffer, FILE** output, int *image_count) {

    if (is_jpg_header(buffer)) {
        if (! start_new_jpg_file(output, *image_count) ) {
            return false;
        }
        (*image_count)++;
    }
    if (*output) {
        if (fwrite(buffer, sizeof(uint8_t), BUFFER_SIZE, *output) != BUFFER_SIZE) {
            perror("process_buffer fwrite");
        }
    }

    return true;
}

void process_file(const char* file_name) {

    FILE* input = fopen(file_name, "r");
    if (!input) {
        perror("Error opening input file");
        exit(1);
    }

    // read file in 512 chunks. If the first 3 bytes match the signature look at the 4th byte and if it matches,
    // we start a new jpg. Keep reading 512b chunks until we see a signature at the start. This ends the last jpg
    // and starts a new jpg.
    int image_count = 0;
    uint8_t buffer[BUFFER_SIZE];
    FILE* output = NULL;
    size_t read = 0;

    while ( (read = fread(buffer,  sizeof(uint8_t), BUFFER_SIZE, input)) == BUFFER_SIZE ) {
        if ( !process_buffer(buffer, &output, &image_count)) {
            fclose(input);
            exit(1);
        }
    }

    if (read > 0 && read < BUFFER_SIZE) {
        // last read was partial
        printf("partial read\n");
        if ( !process_buffer(buffer, &output, &image_count)) {
            fclose(input);
            exit(1);
        }
    }

    printf("image count: %i\n", image_count);

    fclose(input);
}

int main(int argc, char *argv[])
{
    if ( argc != 2 ) {
        printf("usage: recover <filename>\n");
        exit(1);
    }

    process_file(argv[1]);

    return 0;
}