#include <stdio.h>
#include <stdlib.h>
#include "./headers/image.h"
#include "./headers/sequential_blur.h"
#include "./src/image.c"

int main(void) {
    // default value
    // the idea is that the blur radius should be a square (3x3, 5x5, 7x7...)
    int radius = 1;

    Image input = {0};
    Image output = {0};

    if (loadImage(&input, "input.png") != 0) { // if image not found
        fprintf(stderr, "Error: %s \n", image_error()); // fprintf = printf in java
        return 0;
    }

    // malloc dynamically allocates memory, allowing creation of dynamic array
    // multiply by 3 for three colour values (R G B) for every pixel.
    output.width = input.width;
    output.height = input.height;
    output.channels = 3;
    output.pixels = malloc(output.width * output.height * output.channels);

    sequential_blur(&input, &output, radius);

    // "width * 3" means 3 bytes per pixel here
    image_save_png("output.png", &output);

    // deallocate memory
    image_free(&input);
    free(output.pixels);

    return 0;
}
