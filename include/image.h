#ifndef IMAGE_H
#define IMAGE_H

#include <vector>

// simple image struct that stores the pixel values in CHW-format
typedef struct {
    int w, h, c;
    std::vector<float> data;
} image_t;

image_t make_image(int w, int h, int c);
void clear_image(image_t* m);

void set_pixel(image_t* m, int x, int y, int c, float val);

void draw_box(image_t* m, int x1, int y1, int x2, int y2, float r, float g, float b);

void save_image_png(const image_t& m, const char* filename);

#endif
