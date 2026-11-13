#ifndef OPENMP_BLUR_H
#define OPENMP_BLUR_H

#include "image.h"
#include "blur.h"

void openmp_blur(const Image *src, Image *dst, const BlurConfig *blur);
#endif
