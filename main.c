#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "./headers/blur.h"
#include "./headers/cuda_blur.h"
#include "./headers/image.h"
#include "./headers/metrics.h"
#include "./headers/openmp_blur.h"
#include "./headers/sequential_blur.h"
#include "./headers/tui.h"

int main(void) {
    BlurConfig blur = {0};

    // the metrics structs live outside the loop, so measurements
    // from earlier runs stay available for later comparisons
    Metrics seq_metrics = {0};
    Metrics openmp_metrics = {0};
#ifdef USE_CUDA
    Metrics cuda_metrics = {0};
#endif

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
            // sequential baseline, timed only: its pixels are overwritten
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

#ifdef USE_CUDA
        case 4:
            // same pattern as OpenMP: timed sequential baseline first,
            // then the GPU result, which is the one that gets saved
            seq_metrics.start = omp_get_wtime();
            sequential_blur(&input, &output, &blur);
            seq_metrics.end = omp_get_wtime();
            seq_metrics.numberOfThreads = 1;

            cuda_metrics.start = omp_get_wtime();
            if (cuda_blur(&input, &output, &blur) != 0) {
                image_free(&input);
                free(output.pixels);
                return 1;
            }
            cuda_metrics.end = omp_get_wtime();
            // GPU thread count is not comparable to CPU threads
            cuda_metrics.numberOfThreads = 1;

            processOutput(&seq_metrics, &cuda_metrics);
            break;
#else
        case 4:
            // the CUDA sources exist, but this binary was built without them
            fprintf(stderr, "This build has no CUDA support. Run build_cuda.bat to rebuild.\n");
            image_free(&input);
            free(output.pixels);
            return 1;
#endif

        /*
        case 3:
            mpi_metrics.start = omp_get_wtime();
            mpi_blur(&input, &output, &blur);
            mpi_metrics.end = omp_get_wtime();
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
