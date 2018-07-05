#ifndef FILTER_IMAGE_H
#define FILTER_IMAGE_H

#include "image.h"

// different utility functions for creating filters

typedef enum {
    EMBOSS,
    HIGHPASS,
    BOX,
    HORIZONTAL,
    VERTICAL,
    RIGHT_DIAGONAL,
    LEFT_DIAGONAL,
    GX,
    GY,
    SHARPEN,
    SMOOTHEN,
    GAUSSIAN,
} filter_type_t;

filter_type_t get_filter_type(const char* s);
image_t get_filter(filter_type_t filter_type);

image_t make_emboss_filter();
image_t make_highpass_filter();
image_t make_box_filter(int w = 3);

//line detection
image_t make_horizontal_filter();
image_t make_vertical_filter();
image_t make_right_diag_filter();
image_t make_left_diag_filter();

// sobel edge detection 
image_t make_gx_filter();
image_t make_gy_filter();

image_t make_sharpen_filter();
image_t make_smoothing_filter();
image_t make_gaussian_filter(float sigma = 0.5f);

#endif
