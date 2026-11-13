#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "./headers/blur.h"
#include "./headers/image.h"
#include "./headers/sequential_blur.h"
#include "./headers/tui.h"
#include "./headers/openmp_blur.h"
#include "./headers/metrics.h"

int main(void) {
    BlurConfig blur = {0};

    // the metrics structs live outside the loop, so measurements
    // from earlier runs stay available for later comparisons
    Metrics seq_metrics = {0};
    Metrics openmp_metrics = {0};
    Metrics mpi_metrics = {0};
    Metrics cuda_metrics = {0};

    while (1) {
        if (tui_configure(&blur) != 0) {
            return 0;
        }

        // the TUI sets implementation to 0 when the user chooses to exit
        if (blur.implementation == 0) {
            break;
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
            seq_metrics.start = omp_get_wtime();
            sequential_blur(&input, &output, &blur);
            seq_metrics.end = omp_get_wtime();
            seq_metrics.numberOfThreads = 1;

            calcProcessingTime(&seq_metrics);
            metrics_summary(&seq_metrics, NULL);
            break;

        case 2:
            // sequential baseline, timed only — its pixels are overwritten
            // by the parallel run below, so the saved output is the parallel one
            seq_metrics.start = omp_get_wtime();
            sequential_blur(&input, &output, &blur);
            seq_metrics.end = omp_get_wtime();
            seq_metrics.numberOfThreads = 1;

            openmp_metrics.start = omp_get_wtime();
            openmp_blur(&input, &output, &blur);
            openmp_metrics.end = omp_get_wtime();
            openmp_metrics.numberOfThreads = omp_get_max_threads();

            processOutput(&seq_metrics, &openmp_metrics);
            break;

        /*
        case 3:
            mpi_metrics.start = omp_get_wtime();
            mpi_blur(&input, &output, &blur);
            mpi_metrics.end = omp_get_wtime();
            break;

        case 4:
            cuda_metrics.start = omp_get_wtime();
            cuda_blur(&input, &output, &blur);
            cuda_metrics.end = omp_get_wtime();
            break;
        */
        default:
            fprintf(stderr, "That implementation is not available yet.\n");
            image_free(&input);
            free(output.pixels);
            return 1;
        }

        // "width * 3" means 3 bytes per pixel here
        if (image_save_png(blur.output_path, &output) != 0) {
            // error details already printed by image_save_png
            image_free(&input);
            free(output.pixels);
            return 1;
        }

        // deallocate memory
        image_free(&input);
        free(output.pixels);
    }

    return 0;
}
