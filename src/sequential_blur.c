#include "../headers/sequential_blur.h"
#include "../headers/image.h"
#include "./image.c"

void sequential_blur(const Image *src, Image *dst, int radius) {
    int width = src->width;
    int height = src->height;
    const unsigned char *image = src->pixels;
    unsigned char *output = dst->pixels;

    // two for loops, for every pixel in row and column
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            int sum_r = 0, sum_g = 0, sum_b = 0;

            // dx and dy represents offset, affected by radius
            // start from -radius, as we need to read a targeted pixel (image[x*y]) and the neighbouring pixels around it
            for (int dx = -radius; dx <= radius; dx++) {
                for (int dy = -radius; dy <= radius; dy++) {
                    int offset_x = x + dx;
                    int offset_y = y + dy;

                    // ensure pixel traversal stays within the image and not exceed image border
                    if (offset_x < 0) offset_x = 0;
                    if (offset_x >= width) offset_x = width - 1;
                    if (offset_y < 0) offset_y = 0;
                    if (offset_y >= height) offset_y = height - 1;

                    int offset_pixel = offset_y * width + offset_x;

                    // multiply by 3 since there are three values in colours: R G B.
                    int offset_colour = offset_pixel * 3;

                    // sum up rgb values of that pixel
                    sum_r += image[offset_colour];
                    sum_g += image[offset_colour + 1];
                    sum_b += image[offset_colour + 2];
                }
            }

            // calculate average of rgb values
            int average_r = sum_r / 9;
            int average_g = sum_g / 9;
            int average_b = sum_b / 9;

            // store rgb values for one pixel
            int output_pixel = (y * width + x) * 3;
            output[output_pixel] = average_r;
            output[output_pixel + 1] = average_g;
            output[output_pixel + 2] = average_b;
        }
    }
}
