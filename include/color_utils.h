#ifndef COLOR_UTILS_H
#define COLOR_UTILS_H

#include <vector>

#include "rng.h"

typedef struct {
    union {
        float data[4];
        struct { float r, g, b, a; };
    };
} color_t;

color_t hsv_to_rgb(float h, float s, float v);

// https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
std::vector<color_t> get_colors(const int n);

#endif
