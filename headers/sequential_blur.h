#ifndef SEQUENTIAL_BLUR_H
#define SEQUENTIAL_BLUR_H

#include "image.h"
#include "blur.h"

void sequential_blur(const Image *src, Image *dst, const BlurConfig *blur);

#endif
