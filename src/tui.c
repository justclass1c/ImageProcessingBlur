#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../headers/image.h"
#include "../headers/tui.h"

static const char *implementation_name(int option) {
    switch (option) {
    case 1: return "sequential";
    case 2: return "openmp";
    case 3: return "mpi";
    case 4: return "cuda";
    default: return "unknown";
    }
}

int tui_configure(BlurConfig *config) {
    int imgIndex, option, strength;
    char imageNames[MAX_IMAGES][MAX_NAME_LENGTH];
    int count = getImageNames(imageNames);

    if (count <= 0) {
        if (count == 0) {
            fprintf(stderr, "No images found.\n");
        }
        return 1;
    }

    printf("Pick a corresponding image number: ");
    if (scanf("%d", &imgIndex) != 1) {
        return 1;
    }

    if (imgIndex < 1 || imgIndex > count) {
        fprintf(stderr, "Invalid image number.\n");
        return 1;
    }

    printf("Pick an implementation:\n");
    printf("1. Sequential\n");
    printf("2. OpenMP\n");
    printf("3. MPI\n");
    printf("4. CUDA\n");
    printf("> ");
    if (scanf("%d", &option) != 1) {
        return 1;
    }

    if (option < 1 || option > 4) {
        fprintf(stderr, "Invalid implementation.\n");
        return 1;
    }

    printf("Configure blur strength: ");
    if (scanf("%d", &strength) != 1) {
        return 1;
    }

    if (strength < 1) {
        fprintf(stderr, "Strength must be at least 1.\n");
        return 1;
    }

    snprintf(config->input_path, sizeof(config->input_path), "images/%s", imageNames[imgIndex - 1]);

    // current local time, formatted as <dd-mm-yy>_<hh:mm>
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    if (local == NULL) {
        fprintf(stderr, "Failed to get local time.\n");
        return 1;
    }

    char timestamp[32];
    if (strftime(timestamp, sizeof(timestamp), "%d-%m-%y_%H:%M", local) == 0) {
        fprintf(stderr, "Failed to format timestamp.\n");
        return 1;
    }

    // strip the file extension from the image name, so "input.png" becomes "input"
    const char *image_name = imageNames[imgIndex - 1];
    const char *dot = strrchr(image_name, '.');
    size_t base_length = (dot != NULL && dot != image_name)
                         ? (size_t)(dot - image_name)
                         : strlen(image_name);

    // <image>_<date>_<time>_<implementation>_output.png
    int written = snprintf(config->output_path, sizeof(config->output_path),
                           "output/%.*s_%s_%s_output.png",
                           (int)base_length, image_name, timestamp,
                           implementation_name(option));
    if (written < 0 || (size_t)written >= sizeof(config->output_path)) {
        fprintf(stderr, "Output filename too long.\n");
        return 1;
    }

    config->strength = strength;
    config->implementation = option;

    return 0;
}
