#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

int _main(int argc, char *argv[])
{


    // Define allowable filters
    char *filters = "bgrs";

    // Get filter flag and check validity
    char filter = getopt(argc, argv, filters);
    if (filter == '?')
    {
        printf("Invalid filter.\n");
        return 1;
    }

    // Ensure only one filter
    if (getopt(argc, argv, filters) != -1)
    {
        printf("Only one filter allowed.\n");
        return 2;
    }

    // Ensure proper usage
    if (argc != optind + 2)
    {
        printf("Usage: ./filter [flag] infile outfile\n");
        return 3;
    }

    printf("Input arguments:\n");
    printf("--------------------\n");
    for (int i = 0; i < argc; ++i) {
        printf("%i: '%s'\n", i, argv[i]);
    }

    // Remember filenames
    char *infile = argv[optind];
    char *outfile = argv[optind + 1];

    // Open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", infile);
        return 4;
    }

    // Open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        printf("Could not create '%s' \n", outfile);
        perror("");
        return 5;
    }

    // Read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // Read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // Ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        printf("Unsupported file format.\n");
        return 6;
    }

    // Get image's dimensions
    int height = abs(bi.biHeight);
    int width = bi.biWidth;

    // Allocate memory for image
    RGBTRIPLE(*image)[width] = calloc(height, width * sizeof(RGBTRIPLE));
    if (image == NULL)
    {
        printf("Not enough memory to store image.\n");
        fclose(outptr);
        fclose(inptr);
        return 7;
    }

    // Determine padding for scanlines
    int padding = (4 - (width * sizeof(RGBTRIPLE)) % 4) % 4;

    // Iterate over infile's scanlines
    for (int i = 0; i < height; i++)
    {
        // Read row into pixel array
        fread(image[i], sizeof(RGBTRIPLE), width, inptr);

        // Skip over padding
        fseek(inptr, padding, SEEK_CUR);
    }

    // Filter image
    printf("***Switching on filter : %c\n", filter);
    switch (filter)
    {
        // Blur
        case 'b':
            blur(height, width, image);
            break;

        // Grayscale
        case 'g':
            grayscale(height, width, image);
            break;

        // Reflection
        case 'r':
            reflect(height, width, image);
            break;

        // Sepia
        case 's':
            sepia(height, width, image);
            break;
    }

    // Write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // Write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // Write new pixels to outfile
    for (int i = 0; i < height; i++)
    {
        // Write row to outfile
        fwrite(image[i], sizeof(RGBTRIPLE), width, outptr);

        // Write padding at end of row
        for (int k = 0; k < padding; k++)
        {
            fputc(0x00, outptr);
        }
    }

    // Free memory for image
    free(image);

    // Close files
    fclose(inptr);
    fclose(outptr);
    return 0;
}

int main(int argc, char *argv[]) {
    char *input_files[] = {"images/courtyard.bmp", "images/stadium.bmp", "images/tower.bmp", "images/yard.bmp"};
    struct filter {
        char *flag;
        char *name;
    };
    struct filter filters[] = {
        {.flag="-b", .name="blur"},
        {.flag="-g", .name="grayscale"},
        {.flag="-r", .name="reverse"},
        {.flag="-s", .name="sepia"}
    };

    for (int file_index = 0; file_index < 4; ++file_index) {
        for (int flag_index = 0; flag_index < 4; ++flag_index) {
            char output_file[255] = {0};
            strcat(output_file, "images_output/");
            strcat(output_file, strchr(input_files[file_index], '/') + 1); // should copy input file name minus 'images/'
            strcat(output_file, "-");
            strcat(output_file, filters[flag_index].name);
            strcat(output_file, ".bmp");

            char *args[4] = {0};
            args[0] = "filter.out";
            args[1] =  filters[flag_index].flag;
            args[2] =  input_files[file_index];
            args[3] =  output_file;
            optind = 1;
            _main(4, args  );
        }
    }

    return 0;


}