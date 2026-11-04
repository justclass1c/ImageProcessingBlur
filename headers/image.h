#ifndef IMAGE_H
#define IMAGE_H

typedef struct {
    int width;
    int height;
    int channels;
    unsigned char *pixels;
} Image;

int image_load(Image *image, const char *path, int desired_channels);
void image_save_png(const char *path, const Image *image);
void image_free(Image *image);
const char *image_error(void);

#endif
