#ifndef IMAGE_H
#define IMAGE_H

#define MAX_IMAGES 100
#define MAX_NAME_LENGTH 256

typedef struct {
    int width;
    int height;
    int channels;
    unsigned char *pixels;
} Image;

int loadImage(Image *image, const char *path);
int getImageNames(char imageNames[][MAX_NAME_LENGTH]);
void image_save_png(const char *path, const Image *image);
void image_free(Image *image);
const char *image_error(void);

#endif
