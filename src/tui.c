#include <stdio.h>
#include <stdlib.h>
#include "../headers/image.h"
#include "../headers/tui.h"

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
    snprintf(config->output_path, sizeof(config->output_path), "output.png");
    config->strength = strength;
    config->implementation = option;

    return 0;
}
