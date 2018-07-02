#ifndef IMAGE_H
#define IMAGE_H

#include <vector>

// simple image struct that stores the pixel values in CHW-format
typedef struct {
    int w, h, c;
    std::vector<float> data;
} image_t;

image_t make_image(int w, int h, int c);
image_t make_image_grayscale(int w, int h);
image_t make_image_colored(int w, int h);
image_t make_image_from_chw_bytes(int w, int h, int c, unsigned char* data);
image_t make_image_from_hwc_bytes(int w, int h, int c, unsigned char* data);

void set_pixel(image_t* m, int x, int y, int c, float val);
float get_pixel(const image_t& m, int x, int y, int c);
float get_pixel_extend(const image_t& m, int x, int y, int c);

image_t copy_image(const image_t& m);
void copy_image(const image_t& src, image_t* dst);
void clear_image(image_t* m);

void scale_image(image_t* m, float s);
void translate_image(image_t* m, float s);
void fill_image(image_t* m, float s);

void l1_normalize(image_t* im);
void l2_normalize(image_t* im);

void threshold_image(const image_t& in_rgb, image_t* out_gray, float thresh);
void threshold_image(const image_t& in_rgb, image_t* out_gray, float rt, float gt, float bt, float dt);

void draw_box(image_t* m, int x1, int y1, int x2, int y2, float r, float g, float b);

// Get HWC(channels interleaved) bytes from CHW(channels separate) float image
std::vector<unsigned char> get_hwc_bytes(const image_t& m);

image_t load_image(const char* filename, int num_channels = 3);
image_t load_image_rgb(const char* filename);
image_t load_image_grayscale(const char* filename);

void save_image_png(const image_t& m, const char* filename);
void save_image_jpg(const image_t& m, const char* filename, int quality = 100);

void convolve_image(const image_t& in, const image_t& kernel, image_t* out, bool preserve);

#endif
