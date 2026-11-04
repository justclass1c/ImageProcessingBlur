#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb_image.h"
#include "../stb_image_write.h"
#include "../headers/image.h"

int image_load(Image *image, const char *path, int desired_channels) {
    int file_channels = 0;

    image->pixels = stbi_load(path, &image->width, &image->height,
                              &file_channels, desired_channels);
    image->channels = desired_channels;

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

const char *image_error(void) {
    return stbi_failure_reason();
}
