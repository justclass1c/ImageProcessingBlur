#include <stdio.h>
#include <dirent.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb_image.h"
#include "../stb_image_write.h"
#include "../headers/image.h"

int loadImage(Image *image, const char *path) {
    int file_channels = 0;

    image->pixels = stbi_load(path, &image->width, &image->height, &file_channels, 3);
    image->channels = 3;

    return image->pixels == NULL;
}

void image_save_png(const char *path, const Image *image) {
    stbi_write_png(path, image->width, image->height, image->channels,
                   image->pixels, image->width * image->channels);
}

void image_free(Image *image) {
    stbi_image_free(image->pixels);
    image->pixels = NULL;
}

int getImageNames(char imageNames[][MAX_NAME_LENGTH]) {
    DIR *dir = opendir("images");

    if (!dir) {
        perror("opendir"); // print error with not found
        return -1;
    }

    struct dirent *entry;
    int imageCount = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (imageCount >= MAX_IMAGES) break;

        // skip . and .. directories
        if (entry -> d_name[0] == '.') {
            continue;
        }

        int character;
        for (character = 0; character < MAX_NAME_LENGTH - 1; character++) {
            imageNames[imageCount][character] = entry -> d_name[character];
            if (entry -> d_name[character] == '\0') break;
        }
        imageNames[imageCount][character] = '\0';

        imageCount++;
    }

    for (int i = 0; i < imageCount; i++) {
        printf("%d. %s\n", i + 1, imageNames[i]);
    }

    closedir(dir);
    return imageCount;
}

const char *image_error(void) {
    return stbi_failure_reason();
}
