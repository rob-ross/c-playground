#include "helpers.h"

#include <stdio.h>
#include <string.h>
#include <tgmath.h>

#define min( a, b) ((a) < (b) ? (a) : (b))
#define max( a, b) ((a) > (b) ? (a) : (b))

// Convert image to grayscale
// modifies image in-place
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    // set each pixel's color values to the average of their initial color values
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            RGBTRIPLE pixel = image[i][j];
            int average = round( ((pixel.rgbtBlue + pixel.rgbtGreen + pixel.rgbtRed)) / 3.0);
            image[i][j].rgbtBlue = average;
            image[i][j].rgbtGreen = average;
            image[i][j].rgbtRed = average;
        }
    }
}

// Convert image to sepia in-place
// modifies image array
void sepia(int height, int width, RGBTRIPLE image[height][width])
{
    // for each color we reduce the total blues and reds and increase the greens
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            RGBTRIPLE pixel = image[i][j];
            int red = pixel.rgbtRed;
            int green = pixel.rgbtGreen;
            int blue = pixel.rgbtBlue;
            image[i][j].rgbtRed   = min( round(.393 * red + .769 * green + .189 * blue), 255);
            image[i][j].rgbtGreen = min( round(.349 * red + .686 * green + .168 * blue), 255);
            image[i][j].rgbtBlue  = min( round(.272 * red + .534 * green + .131 * blue), 255);
        }
    }
}

// Reflect image horizontally, aka around the vertical axis
void reflect(int height, int width, RGBTRIPLE image[height][width]) {

    for (int i = 0; i < height; ++i) {
        int left = 0;
        int right = width;

        while ( left < right) {
            const RGBTRIPLE temp = image[i][left];
            image[i][left] = image[i][right];
            image[i][right] = temp;
            left++;
            right--;
        }
    }
}

void display_rgb_triple(int r, int g, int b) {
    printf("(r=%i, g=%i, b=%i)", r, g, b);
}

RGBTRIPLE average_neighbors(const int row, const int col,
                            const int height, const int width, RGBTRIPLE image[height][width]) {


    float num_samples = 0; // max 9, min 4 (for the corner pixels). float to force float division

    struct sum_s { int red; int green; int blue;} sum =  {0};

    const int start_row = max(0, row - 1);
    const int end_row   = min(height - 1, row + 1);
    const int start_col = max(0, col - 1);
    const int end_col   = min(width - 1, col + 1);
    for (int i = start_row;     i <= end_row   ; i++) {
        for (int j = start_col; j <= end_col;    ++j) {
            num_samples++;
            sum.red   += image[i][j].rgbtRed;
            sum.green += image[i][j].rgbtGreen;
            sum.blue  += image[i][j].rgbtBlue;
        }
    }

    sum.red   = min( round( sum.red   / num_samples ), 255);
    sum.green = min( round( sum.green / num_samples ), 255);
    sum.blue  = min( round( sum.blue  / num_samples ), 255);

    return (RGBTRIPLE){.rgbtRed = sum.red, .rgbtGreen = sum.green, .rgbtBlue = sum.blue};
}

// Blur image
void blur(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE copy[height][width];
    memcpy(copy, image, height * width * sizeof(RGBTRIPLE));

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            image[i][j] = average_neighbors(i, j, height, width, copy);
        }
    }
}
