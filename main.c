#include <stdio.h>
#include <stdlib.h>
#include "./headers/blur.h"
#include "./headers/image.h"
#include "./headers/sequential_blur.h"
#include "./headers/tui.h"

int main(void) {
    BlurConfig blur = {0};

    if (tui_configure(&blur) != 0) {
        return 0;
    }

    calculateBlurParams(&blur);

    Image input = {0};
    Image output = {0};

    if (loadImage(&input, blur.input_path) != 0) { // if image not found
        fprintf(stderr, "Error: %s \n", image_error()); // fprintf = printf in java
        return 0;
    }

    // malloc dynamically allocates memory, allowing creation of dynamic array
    // multiply by 3 for three colour values (R G B) for every pixel.
    output.width = input.width;
    output.height = input.height;
    output.channels = 3;
    output.pixels = malloc(output.width * output.height * output.channels);

    switch (blur.implementation) {
    case 1:
        sequential_blur(&input, &output, &blur);
        break;
    /*
    case 2:
        openmp_blur(&input, &output, &blur);
        break;

    case 3:
        mpi_blur(&input, &output, &blur);
        break;

    case 4:
        cuda_blur(&input, &output, &blur);
        break;
    */
    default:
        fprintf(stderr, "That implementation is not available yet.\n");
        image_free(&input);
        free(output.pixels);
        return 1;
    }

    // "width * 3" means 3 bytes per pixel here
    image_save_png(blur.output_path, &output);

    // deallocate memory
    image_free(&input);
    free(output.pixels);

    return 0;
}
