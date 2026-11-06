#include <stdint.h>
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

int getImageNames() {
    DIR *dir = opendir("image");

    if (!dir) {
        perror("opendir"); // print error with not found
        return 1;
    }

    struct dirent *entry;
    int imageCount = 0;

    while ((entry = readdir(dir)) != NULL) {
        // skip . and .. directories
        if (entry -> d_name[0] == '.') {
            continue;
        }

        imageCount++;

        printf("%d. %s", imageCount, entry->d_name);
    }

    closedir(dir);
    return 0;
}

const char *image_error(void) {
    return stbi_failure_reason();
}
