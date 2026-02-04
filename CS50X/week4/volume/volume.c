// Modifies the volume of an audio file
// example wav file:
//  curl -O https://cdn.cs50.net/2026/x/psets/4/volume.zip
// make: clang volume.c -o volume

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>


//make: clang volume.c -o volume.out

// Number of bytes in .wav header
const int HEADER_SIZE = 44;


// multiples each member of the buffer by factor amount, rounding the result
// modifies the buffer in-place.
void apply_factor(int16_t *buffer, const size_t size, const double factor) {
    for (int i = 0; i < size; ++i) {
        buffer[i] = round(buffer[i] * factor); // can clip.
    }
}

void handle_error(FILE* input, FILE* output, char *message) {
    perror(message);
    fclose(input);
    fclose(output);
    exit(1);
}

int main(int argc, char *argv[])
{

    // Check command-line arguments
    if (argc != 4)
    {
        printf("Usage: ./volume input.wav output.wav factor\n");
        return 1;
    }

//R"(input.wav)")  // what is a raw character literal?
    if ( strcmp(argv[2], "input.wav") == 0) {
        printf("Cannot write to 'input.wav'\n");
        return 1;
    }

    // Open files and determine scaling factor
    FILE *input = fopen(argv[1], "r");
    if (input == NULL)
    {
        printf("Could not open input file '%s'.\n", argv[1]);
        return 1;
    }

    FILE *output = fopen(argv[2], "w");
    if (output == NULL)
    {
        printf("Could not open output file '%s'.\n", argv[2]);
        return 1;
    }

    float factor = atof(argv[3]);

    //  Copy header from input file to output file
    uint8_t header_buffer[HEADER_SIZE];
    size_t read_size = fread(header_buffer, HEADER_SIZE, 1, input); // should == 1 for 1 element read
    if (read_size == 0 || ferror(input) != 0) {
        handle_error(input, output,"Error reading header");
        return 1;
    }

    size_t write_size = fwrite(header_buffer, HEADER_SIZE, 1, output);
    if (write_size == 0 || ferror(output) != 0) {
        handle_error(input, output,"Error writing header");
        return 1;
    }

    // Read samples from input file and write updated data to output file

    const size_t WAVE_BUFFER_SIZE = 1024;
    int16_t wave_buffer[WAVE_BUFFER_SIZE]; // we probably want to read as signed int to handle clipping if factor causes overflow/underflow

    while ( (read_size = fread(wave_buffer, sizeof(int16_t), WAVE_BUFFER_SIZE, input)) == WAVE_BUFFER_SIZE) {

        // multiply buffer by factor
        apply_factor(wave_buffer, read_size, factor);
        //write modified buffer to output
        write_size = fwrite(wave_buffer, sizeof(int16_t), read_size, output);
        if ( write_size != read_size || ferror(output) != 0) {
            handle_error(input, output,"Error writing output data.");
            return 1;
        }

    }

    // write remaining buffer

    write_size = fwrite(wave_buffer, sizeof(int16_t), read_size, output);

    // multiply by factor
    apply_factor(wave_buffer, read_size, factor);

    if ( write_size != read_size || ferror(output) != 0) {
        handle_error(input, output,"Error writing output data.");
        return 1;
    }

    // Close files
    fclose(input);
    fclose(output);
}
