#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "./headers/blur.h"
#include "./headers/cuda_blur.h"
#include "./headers/image.h"
#include "./headers/metrics.h"
#include "./headers/mpi_blur.h"
#include "./headers/openmp_blur.h"
#include "./headers/sequential_blur.h"
#include "./headers/tui.h"

// in an MPI build only rank 0 runs this
static int run_app(void) {
    BlurConfig blur = {0};

    // the metrics structs live outside the loop, so measurements
    // from earlier runs stay available for later comparisons
    Metrics seq_metrics = {0};
    Metrics openmp_metrics = {0};
#ifdef USE_MPI
    Metrics mpi_metrics = {0};
#endif
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

#ifdef USE_MPI
        case 3: {
            // separate baseline buffer, so it can be compared with the MPI result
            size_t bytes = (size_t)output.width * output.height * output.channels;
            Image baseline = output;
            baseline.pixels = malloc(bytes);
            if (baseline.pixels == NULL) {
                fprintf(stderr, "Error: out of memory.\n");
                image_free(&input);
                free(output.pixels);
                return 1;
            }

            seq_metrics.start = omp_get_wtime();
            sequential_blur(&input, &baseline, &blur);
            seq_metrics.end = omp_get_wtime();
            seq_metrics.numberOfThreads = 1;

            // timed including the send and gather
            mpi_metrics.start = omp_get_wtime();
            if (mpi_blur(&input, &output, &blur) != 0) {
                free(baseline.pixels);
                image_free(&input);
                free(output.pixels);
                return 1;
            }
            mpi_metrics.end = omp_get_wtime();
            // processes, not threads
            mpi_metrics.numberOfThreads = mpi_process_count();

            printf("MPI processes: %d\n", mpi_process_count());
            if (memcmp(baseline.pixels, output.pixels, bytes) == 0) {
                printf("Correctness check: MPI output is identical to the sequential output.\n");
            } else {
                fprintf(stderr, "Correctness check FAILED: MPI output differs from the sequential output.\n");
            }
            free(baseline.pixels);

            processOutput(&seq_metrics, &mpi_metrics);
            break;
        }
#else
        case 3:
            fprintf(stderr, "This build has no MPI support. Rebuild with: make clean, then make USE_MPI=1\n");
            image_free(&input);
            free(output.pixels);
            return 1;
#endif

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

int main(int argc, char **argv) {
#ifdef USE_MPI
    // ranks 1..N-1 are workers and never reach the menu
    if (mpi_start(&argc, &argv) != 0) {
        mpi_worker_loop();
        mpi_shutdown();
        return 0;
    }

    // stdout is a pipe under mpiexec; unbuffered so prompts show before input
    setvbuf(stdout, NULL, _IONBF, 0);
#else
    (void)argc;
    (void)argv;
#endif

    int status = run_app();

#ifdef USE_MPI
    mpi_shutdown();
#endif

    return status;
}
