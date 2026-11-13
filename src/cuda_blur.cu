#include <stdio.h>
#include <cuda_runtime.h>
#include "../headers/cuda_blur.h"

/*
 * One CUDA thread = one output pixel, exactly like one iteration of the
 * sequential loops in sequential_blur.c.
 *
 * The algorithm is the same box blur:
 *   radius       = diameter / 2
 *   sample_count = diameter * diameter
 *   for every pixel: sum every RGB value inside the (2r+1)x(2r+1) window,
 *   clamping coordinates to the image border (edge pixels are repeated),
 *   then divide the sums by sample_count with integer division.
 */
__global__ void box_blur_kernel(const unsigned char *__restrict__ input,
                                unsigned char *__restrict__ output,
                                int width, int height,
                                int radius, int sample_count) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    // ignore threads that fall outside of the image
    if (x >= width || y >= height) return;

    int sum_r = 0, sum_g = 0, sum_b = 0;

    // dx and dy are offsets around the target pixel
    for (int dy = -radius; dy <= radius; dy++) {
        int offset_y = y + dy;

        // clamp to the image border, same as the sequential version
        if (offset_y < 0) offset_y = 0;
        if (offset_y >= height) offset_y = height - 1;

        const unsigned char *row = input + (size_t)offset_y * width * 3;

        for (int dx = -radius; dx <= radius; dx++) {
            int offset_x = x + dx;

            if (offset_x < 0) offset_x = 0;
            if (offset_x >= width) offset_x = width - 1;

            int colour = offset_x * 3;

            // __ldg: read through the read-only data cache
            sum_r += __ldg(row + colour);
            sum_g += __ldg(row + colour + 1);
            sum_b += __ldg(row + colour + 2);
        }
    }

    int output_pixel = (y * width + x) * 3;
    output[output_pixel] = (unsigned char)(sum_r / sample_count);
    output[output_pixel + 1] = (unsigned char)(sum_g / sample_count);
    output[output_pixel + 2] = (unsigned char)(sum_b / sample_count);
}

static int cuda_check(cudaError_t status, const char *what) {
    if (status != cudaSuccess) {
        fprintf(stderr, "CUDA error (%s): %s\n", what, cudaGetErrorString(status));
        return 1;
    }
    return 0;
}

int cuda_blur(const Image *src, Image *dst, const BlurConfig *blur) {
    int width = src->width;
    int height = src->height;
    size_t bytes = (size_t)width * height * 3;

    unsigned char *device_input = NULL;
    unsigned char *device_output = NULL;
    dim3 block, grid;
    int result = 1;

    if (cuda_check(cudaMalloc((void **)&device_input, bytes), "cudaMalloc(input)")) {
        goto cleanup;
    }
    if (cuda_check(cudaMalloc((void **)&device_output, bytes), "cudaMalloc(output)")) {
        goto cleanup;
    }

    if (cuda_check(cudaMemcpy(device_input, src->pixels, bytes, cudaMemcpyHostToDevice),
                   "cudaMemcpy(input)")) {
        goto cleanup;
    }

    // 16 x 16 threads per block (a block covers a 16 x 16 pixel tile)
    block = dim3(16, 16);
    grid = dim3((width + block.x - 1) / block.x,
                (height + block.y - 1) / block.y);

    box_blur_kernel<<<grid, block>>>(device_input, device_output,
                                     width, height,
                                     blur->diameter / 2, blur->sample_count);

    if (cuda_check(cudaGetLastError(), "kernel launch")) {
        goto cleanup;
    }
    if (cuda_check(cudaDeviceSynchronize(), "kernel execution")) {
        goto cleanup;
    }

    if (cuda_check(cudaMemcpy(dst->pixels, device_output, bytes, cudaMemcpyDeviceToHost),
                   "cudaMemcpy(output)")) {
        goto cleanup;
    }

    result = 0;

cleanup:
    cudaFree(device_output);
    cudaFree(device_input);
    return result;
}
