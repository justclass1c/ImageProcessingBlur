#ifndef BLUR_H
#define BLUR_H

#define MAX_PATH_LENGTH 512

typedef struct {
    char input_path[MAX_PATH_LENGTH];
    char output_path[MAX_PATH_LENGTH];
    int strength;      // user input
    int diameter;      // kernel side length: 2 * half_width + 1
    int sample_count;  // diameter * diameter
    int implementation;
} BlurConfig;

void calculateBlurParams(BlurConfig *blur);

#endif
