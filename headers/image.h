#ifndef IMAGE_H
#define IMAGE_H

typedef struct {
    int width;
    int height;
    int channels;
    unsigned char *pixels;
} Image;

int loadImage(Image *image, const char *path);
void image_save_png(const char *path, const Image *image);
void image_free(Image *image);
int getImageNames();
const char *image_error(void);

#endif
