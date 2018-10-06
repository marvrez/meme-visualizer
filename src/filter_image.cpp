#include "filter_image.h"

#include <math.h>

filter_type_t get_filter_type(const char* s) 
{
    if(strcmp(s, "emboss") == 0) return EMBOSS;
    if(strcmp(s, "highpass") == 0) return HIGHPASS;
    if(strcmp(s, "box") == 0) return BOX;
    if(strcmp(s, "horizontal") == 0) return HORIZONTAL;
    if(strcmp(s, "vertical") == 0) return VERTICAL;
    if(strcmp(s, "right diagonal") == 0) return RIGHT_DIAGONAL;
    if(strcmp(s, "left diagonal") == 0) return LEFT_DIAGONAL;
    if(strcmp(s, "gx") == 0) return GX;
    if(strcmp(s, "gy") == 0) return GY;
    if(strcmp(s, "sharpen") == 0) return SHARPEN;
    if(strcmp(s, "smoothen") == 0) return SMOOTHEN;
    if(strcmp(s, "gaussian") == 0) return GAUSSIAN;
    return GAUSSIAN;
}

image_t get_filter(filter_type_t filter_type) 
{
    switch(filter_type) {
        case EMBOSS:
            return make_emboss_filter();
        case HIGHPASS:
            return make_highpass_filter();
        case BOX:
            return make_box_filter();
        case HORIZONTAL:
            return make_horizontal_filter();
        case VERTICAL:
            return make_vertical_filter();
        case RIGHT_DIAGONAL:
            return make_right_diag_filter();
        case LEFT_DIAGONAL:
            return make_left_diag_filter();
        case GX:
            return make_gx_filter();
        case GY:
            return make_gy_filter();
        case SHARPEN:
            return make_sharpen_filter();
        case SMOOTHEN:
            return make_smoothing_filter();
        case GAUSSIAN:
            return make_gaussian_filter();
    }
    return make_image(3,3,1);
}

image_t make_emboss_filter()
{
    image_t f = make_image(3,3,1);
    f.data[0] = -2; f.data[1] = -1; f.data[2] = 0;
    f.data[3] = -1; f.data[4] =  1; f.data[5] = 1;
    f.data[6] =  0; f.data[7] =  1; f.data[8] = 2;
    return f;
}

image_t make_horizontal_filter()
{
    image_t f = make_image(3,3,1);
    f.data[0] = -1; f.data[1] = -1; f.data[2] = -1;
    f.data[3] =  2; f.data[4] =  2; f.data[5] =  2;
    f.data[6] = -1; f.data[7] = -1; f.data[8] = -1;
    return f;
}

image_t make_vertical_filter()
{
    image_t f = make_image(3,3,1);
    f.data[0] = -1; f.data[1] = 0; f.data[2] = -1;
    f.data[3] = -1; f.data[4] = 0; f.data[5] = -1;
    f.data[6] = -1; f.data[7] = 0; f.data[8] = -1;
    return f;
}

image_t make_right_diag_filter()
{
    image_t f = make_image(3,3,1);
    f.data[0] = -1; f.data[1] = -1; f.data[2] =  2;
    f.data[3] = -1; f.data[4] =  2; f.data[5] = -1;
    f.data[6] =  2; f.data[7] = -1; f.data[8] = -1;
    return f;
}

image_t make_left_diag_filter()
{
    image_t f = make_image(3,3,1);
    f.data[0] =  2; f.data[1] = -1; f.data[2] = -1;
    f.data[3] = -1; f.data[4] =  2; f.data[5] = -1;
    f.data[6] = -1; f.data[7] = -1; f.data[8] =  2;
    return f;
}

image_t make_highpass_filter()
{
    image_t f = make_image(3,3,1);
    f.data[0] =  0; f.data[1] = -1; f.data[2] =  0;
    f.data[3] = -1; f.data[4] =  4; f.data[5] = -1;
    f.data[6] =  0; f.data[7] = -1; f.data[8] =  0;
    return f;
}

image_t make_box_filter(int w)
{
    image_t f = make_image(w,w,1);
    int i;
    for(i = 0; i < w*w; ++i){
        f.data[i] = 1;
    }
    l1_normalize(&f);
    return f;
}

image_t make_gx_filter()
{
    image_t f = make_image(3,3,1);
    f.data[0] = -1; f.data[1] = 0; f.data[2] = 1;
    f.data[3] = -2; f.data[4] = 0; f.data[5] = 2;
    f.data[6] = -1; f.data[7] = 0; f.data[8] = 1;
    return f;
}

image_t make_gy_filter()
{
    image_t f = make_image(3,3,1);
    f.data[0] = -1; f.data[1] = -2; f.data[2] = -1;
    f.data[3] =  0; f.data[4] =  0; f.data[5] =  0;
    f.data[6] =  1; f.data[7] =  2; f.data[8] =  1;
    return f;
}

image_t make_sharpen_filter()
{
    image_t f = make_image(3,3,1);
    f.data[0] =  0; f.data[1] = -1; f.data[2] =  0;
    f.data[3] = -1; f.data[4] =  5; f.data[5] = -1;
    f.data[6] =  0; f.data[7] = -1; f.data[8] =  0;
    return f;
}

image_t make_smoothing_filter()
{
    image_t f = make_image(3,3,1);
    float val = 1.f / 9.f;
    f.data[0] = val; f.data[1] = val; f.data[2] = val;
    f.data[3] = val; f.data[4] = val; f.data[5] = val;
    f.data[6] = val; f.data[7] = val; f.data[8] = val;
    return f;
}

image_t make_gaussian_filter(float sigma)
{
    int w = ((int)(6*sigma)) | 1;
    image_t f = make_image(w, w, 1);

    const float c = 1.f / 2*M_PI*sigma*sigma;
    for(int i = 0; i < w; ++i){
        for(int j = 0; j < w; ++j){
            float x = w / 2. - i - .5;
            float y = w / 2. - j - .5;
            float val = c * exp(-(x*x + y*y)/(2*sigma*sigma));
            set_pixel(&f, i, j, 0, val);
        }
    }
    l1_normalize(&f);
    return f;
}
