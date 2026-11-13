#ifndef CUDA_BLUR_H
#define CUDA_BLUR_H

#include "image.h"
#include "blur.h"

/* nvcc compiles .cu files as C++, main.c is compiled as C.
 * extern "C" keeps the symbol name the same for both, otherwise the
 * linker would look for two differently mangled functions. */
#ifdef __cplusplus
extern "C" {
#endif

/* Same box blur as sequential_blur(), executed on the GPU.
 * Returns 0 on success, non-zero on a CUDA error. */
int cuda_blur(const Image *src, Image *dst, const BlurConfig *blur);

#ifdef __cplusplus
}
#endif

#endif
