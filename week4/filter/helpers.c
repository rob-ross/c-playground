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

struct IndexInits {
    int start_row;
    int end_row;
    int start_col;
    int end_col;
};

struct IndexInits calc_index_inits(const int row, const int col, const int height, const int width) {
    struct IndexInits inits = {
        .start_row = max(0, row - 1),
        .end_row   = min(height - 1, row + 1),
        .start_col = max(0, col - 1),
        .end_col   = min(width - 1, col + 1)
    };
    return inits;
}

RGBTRIPLE average_neighbors(
    const int row, const int col,
    const int height, const int width, RGBTRIPLE image[height][width]) {

    float num_samples = 0; // max 9, min 4 (for the corner pixels). float to force float division
    struct sum_s { int red; int green; int blue;} sum =  {0};

    const struct IndexInits inits = calc_index_inits(row, col, height, width);

    for (int i = inits.start_row;     i <= inits.end_row   ; i++) {
        for (int j = inits.start_col; j <= inits.end_col;    ++j) {
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

const int Gx[3][3] = {
    { -1, 0, 1},
    { -2, 0, 2},
    { -1, 0, 1}
};

const int Gy[3][3] = {
    { -1, -2, -1},
    {  0,  0,  0},
    {  1,  2,  1}
};

RGBTRIPLE sobel_filter( const int row, const int col,
                        const int height, const int width, RGBTRIPLE image[height][width] ) {

    const struct IndexInits inits = calc_index_inits(row, col, height, width);
    struct offsets { int top; int bottom; int left; int right;};

    // offsets are used when the pixel doesn't have a full 8 neighbors, like a corner pixel.
    const struct offsets offsets = {
        .top =    row==0 ? 1 : 0,
        .bottom = row==height-1 ? -1 : 0,
        .left =   col==0 ? 1 : 0,
        .right =  col==width-1 ? -1 : 0
    };
    struct sum_s { int red; int green; int blue;};
    struct sum_s sum_x = {0};
    struct sum_s sum_y = {0};

    for (int i = inits.start_row;     i <= inits.end_row ;   i++) {
        for (int j = inits.start_col; j <= inits.end_col;    j++) {
            sum_x.red   += (image[i][j].rgbtRed   * Gx[i - inits.start_row + offsets.top + offsets.bottom][j - inits.start_col + offsets.left + offsets.right]);
            sum_x.green += (image[i][j].rgbtGreen * Gx[i - inits.start_row + offsets.top + offsets.bottom][j - inits.start_col + offsets.left + offsets.right]);
            sum_x.blue  += (image[i][j].rgbtBlue  * Gx[i - inits.start_row + offsets.top + offsets.bottom][j - inits.start_col + offsets.left + offsets.right]);

            sum_y.red   += (image[i][j].rgbtRed   * Gy[i - inits.start_row + offsets.top + offsets.bottom][j - inits.start_col + offsets.left + offsets.right]);
            sum_y.green += (image[i][j].rgbtGreen * Gy[i - inits.start_row + offsets.top + offsets.bottom][j - inits.start_col + offsets.left + offsets.right]);
            sum_y.blue  += (image[i][j].rgbtBlue  * Gy[i - inits.start_row + offsets.top + offsets.bottom][j - inits.start_col + offsets.left + offsets.right]);
        }
    }

    int red =   min(sqrt(pow(sum_x.red, 2) + pow(sum_y.red, 2)), 255);
    int green = min(sqrt(pow(sum_x.green, 2) + pow(sum_y.green, 2)), 255);
    int blue =  min(sqrt(pow(sum_x.blue, 2) + pow(sum_y.blue, 2)), 255);

    return (RGBTRIPLE){.rgbtRed = red, .rgbtGreen = green, .rgbtBlue = blue};
}

// Detect edges
// modifies image in-place
void edges(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE copy[height][width];
    memcpy(copy, image, height * width * sizeof(RGBTRIPLE));

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            image[i][j] = sobel_filter(i, j, height, width, copy);
        }
    }
}