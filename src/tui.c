#include <stdlib.h>
#include "../headers/image.h"
#include "./image.c"

int tui() {
    int imgIndex, option;

    getImageNames(); 
    scanf("Pick a corresponding image number: %d", &imgIndex);
    // loadImage();

    printf("Pick an implementation:\n");
    printf("1. Sequential\n");
    printf("2. OpenMP\n");
    printf("3. MPI\n");
    printf("4. CUDA\n");
    scanf("> %d", &option);

    // if (option == 1) sequential_blur();
    // if (option == 2) openmpBlur();
    // if (option == 3) mpiBlur();
    // if (option == 4) cudaBlur();
    // else {}

    // allow user to continue instead of exiting program after one image is generated
    return 0;
}
