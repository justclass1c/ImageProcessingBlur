#include "../headers/blur.h"

void calculateBlurParams(BlurConfig *blur) {
    int scaled_strength = blur -> strength * 3;
    int half_width = scaled_strength * scaled_strength;

    blur -> diameter = 2 * half_width + 1;
    blur -> sample_count = blur -> diameter * blur -> diameter;
}
