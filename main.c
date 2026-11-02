#include <stdio.h>
#include <stdlib.h>
// #include "./sequential.c"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

int main() {
    int width, height, channels;

    // default value
    // the idea is that the blur radius should be a square (3x3, 5x5, 7x7...)
    int radius = 1; 
    

    // syntax: unsigned char *variable = stbi_load("filename.jpg/png", &width, &height, &channels, n)
    // stbi_load only returns one value, thus pointers to width, height, channels must be used to obtain their values
    // width = outputs image width in pixels
    // height = output image height in pixels
    // channels = number of image components in image file
    // n = image components (colours) such as greyscale, alpha, rgb, and more.
    // this, however, produces a 2D stream of rgb colour codes.
    unsigned char *image = stbi_load("input.png", &width, &height, &channels, 3);

    if (!image) { // if image not found
        fprintf(stderr, "Error: %s \n", stbi_failure_reason()); // fprintf = printf in java
        return 0;
    }

    // malloc dynamically allocates memory, allowing creation of dynamic array
    // multiply by 3 for three colour values (R G B) for every pixel.
    unsigned char* output = malloc(width * height * 3); 

    // core idea: read colours of one targeted pixel and the neighbouring pixel around it (radius)
    // colour in each pixel is written in 24-bit integers (imagine it converted from colour hex code)
    // this code here shows each pixel's colour bytes, using R, G, B format in decimals
  //for (int i = 0; i < (width * height); i++) {
  //    printf("%u", image[i]);

  //    if (i % 3 == 0) printf("\n");
  //    else printf("  ");
  //}

    // thus, get sum of colours and average it. use that as blur
    int counter = 0;

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

    // "width * 3" means 3 bytes per pixel here
    stbi_write_png("output.png", width, height, 3, output, width * 3);

    // deallocate memory
    stbi_image_free(image);
    free(output);
}
