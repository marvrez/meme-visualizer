#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

image_t make_image(int w, int h, int c)
{
    image_t out = {w, h, c, std::vector<float>(w*h*c, 0.f)};
    return out;
}

image_t make_image_grayscale(int w, int h)
{
    return make_image(w,h,1);
}

image_t make_image_colored(int w, int h)
{
    return make_image(w,h,3);
}

image_t make_image_from_chw_bytes(int w, int h, int c, unsigned char* data)
{
    image_t out = make_image(w,h,c);
    for(int i = 0; i < w*h*c; ++i) {
        out.data[i] = (float)data[i] / 255.f;
    }
    return out;
}

image_t make_image_from_hwc_bytes(int w, int h, int c, unsigned char* data)
{
    image_t m = make_image(w, h, c);
    for(int k = 0; k < c; ++k) {
        for(int j = 0; j < h; ++j) {
            for(int i = 0; i < w; ++i) {
                m.data[i + w*j + w*h*k] = (float)data[k + c*i + c*w*j]/255.f;
            }
        }
    }
    return m;
}

void set_pixel(image_t* m, int x, int y, int c, float val)
{
    if (x < 0 || y < 0 || c < 0 || x >= m->w || y >= m->h || c >= m->c) return;
    m->data[(c * m->h * m->w) + (y * m->w) + x] = val;
}

void add_pixel(image_t* m, int x, int y, int c, float val)
{
    if (x < 0 || y < 0 || c < 0 || x >= m->w || y >= m->h || c >= m->c) return;
    m->data[(c * m->h * m->w) + (y * m->w) + x] += val;
}

float get_pixel(const image_t& m, int x, int y, int c)
{
    assert(x < m.w && y < m.h && c < m.c);
    return m.data[x + y*m.w + c*m.h*m.w];
}

float get_pixel_extend(const image_t& m, int x, int y, int c)
{
    if(x < 0 || x >= m.w || y < 0 || y >= m.h) return 0;
    if(c < 0 || c >= m.c) return 0;
    return get_pixel(m, x, y, c);
}

void clear_image(image_t* m)
{
    m->data = std::vector<float>(m->w*m->h*m->c, 0.f);
}

void scale_image(image_t* m, float s)
{
    for(int i = 0; i < m->h*m->w*m->c; ++i) 
        m->data[i] *= s;
}

void translate_image(image_t* m, float s)
{
    for(int i = 0; i < m->h*m->w*m->c; ++i) 
        m->data[i] += s;
}

void fill_image(image_t* m, float s)
{
    for(int i = 0; i < m->h*m->w*m->c; ++i)
        m->data[i] = s;
}

void threshold_image(const image_t& in_rgb, image_t* out_gray, float thresh)
{
    *out_gray = make_image(in_rgb.w, in_rgb.h, in_rgb.c);
    for(int i = 0; i < in_rgb.w*in_rgb.h*in_rgb.c; ++i) {
        out_gray->data[i] = (in_rgb.data[i] > thresh) ? 1.f : 0.f;
    }
}

void threshold_image(const image_t& in_rgb, image_t* out_gray, float rt, float gt, float bt, float dt)
{
    *out_gray = make_image_grayscale(in_rgb.w, in_rgb.h);
    for (int y = 0; y < in_rgb.h; ++y) {
        for (int x = 0; x < in_rgb.w; ++x) {
            float r = get_pixel(in_rgb,x,y,0), g = get_pixel(in_rgb,x,y,1), b = get_pixel(in_rgb,x,y,2);

            float dr = fabsf(r - rt), dg = fabsf(g - gt), db = fabsf(b - bt);
            float dd = (dr + dg + db) / 3.0f;

            float result = 0.f;
            if (dd < dt) {
                float result_real = (2*r + 1*b + 3*g) / 6.0f;
                result_real *= 1.0f - dd/dt;
                result = result_real < 0 ? 0.f : (result_real > 1.f ? 1.f : result_real);
            }
            set_pixel(out_gray, x, y, 0, result);
        }
    }
}


void copy_image(const image_t& src, image_t* dst)
{
    *dst = { src.w, src.h, src.c, src.data };
}

image_t copy_image(const image_t& m)
{
    return {m.w, m.h, m.c, m.data};
}

void draw_box(image_t* m, int x1, int y1, int x2, int y2, float r, float g, float b)
{
    if(x1 < 0) x1 = 0; if(x1 >= m->w) x1 = m->w - 1;
    if(x2 < 0) x2 = 0; if(x2 >= m->w) x2 = m->w - 1;

    if(y1 < 0) y1 = 0; if(y1 >= m->h) y1 = m->h - 1;
    if(y2 < 0) y2 = 0; if(y2 >= m->h) y2 = m->h - 1;

    for(int i = x1; i <= x2; ++i) {
        set_pixel(m, i, y1, 0, r);
        set_pixel(m, i, y2, 0, r);

        set_pixel(m, i, y1, 1, g);
        set_pixel(m, i, y2, 1, g);

        set_pixel(m, i, y1, 2, b);
        set_pixel(m, i, y2, 2, b);
    }

    for(int i = y1; i <= y2; ++i) {
        set_pixel(m, x1, i, 0, r);
        set_pixel(m, x2, i, 0, r);

        set_pixel(m, x1, i, 1, g);
        set_pixel(m, x2, i, 1, g);

        set_pixel(m, x1, i, 2, b);
        set_pixel(m, x2, i, 2, b);
    }
}

void draw_line(image_t* m, int x1, int y1, int x2, int y2, float r, float g, float b)
{
    int dx = abs(x2-x1), dy = abs(y2-y1);
    bool steep = dy > dx;
    if(steep) { std::swap(x1,y1); std::swap(x2,y2); std::swap(dx,dy); }
    int error = (dx+1) / 2, y_step = (y1 < y2) ? 1 : -1;

    for(int x = x1, y = y1; x <= x2; ++x) {
        int plot_x = steep ? y : x, plot_y = steep ? x : y;

        set_pixel(m, plot_x, plot_y, 0, r);
        set_pixel(m, plot_x, plot_y, 1, g);
        set_pixel(m, plot_x, plot_y, 2, b);

        error -= dy;
        if(error <= 0) { y += y_step; error += dx; }
    }
}

void draw_grid(image_t* m, float x_min, float x_max, float y_min, float y_max, int steps, float r, float g, float b)
{
    for (int i = 0; i <= steps; i++) {
        draw_line(m, x_min, y_min + (y_max-y_min)*i/steps,
                     x_max, y_min + (y_max-y_min)*i/steps,
                     r, g, b);

        draw_line(m, x_min + (x_max-x_min)*i/steps, y_min,
                     x_min + (x_max-x_min)*i/steps, y_max,
                     r, g, b);
    }
}

std::vector<unsigned char> get_hwc_bytes(const image_t& m)
{
    std::vector<unsigned char> bytes(m.c*m.h*m.w, 0);
    for(int k = 0; k < m.c; ++k) {
        for(int i = 0; i < m.w*m.h; ++i) {
            bytes[i*m.c+k] = (unsigned char) (255*m.data[i + k*m.w*m.h]);
        }
    }
    return bytes;
}

image_t load_image(const char* filename, int num_channels)
{
    int w, h, c;
    unsigned char* data = stbi_load(filename, &w, &h, &c, num_channels);
    if (!data) {
        fprintf(stderr, "Cannot load image \"%s\"\nSTB Reason: %s\n", filename, stbi_failure_reason());
        exit(0);
    }
    if(num_channels) c = num_channels;
    image_t img = make_image_from_hwc_bytes(w,h,c,data);
    delete[] data;
    return img;
}

image_t load_image_rgb(const char* filename)
{
    return load_image(filename, 3);
}

image_t load_image_grayscale(const char* filename)
{
    return load_image(filename, 1);
}

void save_image_png(const image_t& m, const char* filename)
{
    char buffer[256];
    sprintf(buffer, "%s.png", filename);
    std::vector<unsigned char> pixels = get_hwc_bytes(m);
    int success = stbi_write_png(buffer, m.w, m.h, m.c, pixels.data(), m.w*m.c);
    if(!success) fprintf(stderr, "Failed to write image %s\n", buffer);
}

void save_image_jpg(const image_t& m, const char* filename, int quality)
{
    char buffer[256];
    sprintf(buffer, "%s.jpg", filename);
    std::vector<unsigned char> pixels = get_hwc_bytes(m);
    int success = stbi_write_jpg(buffer, m.w, m.h, m.c, pixels.data(), quality);
    if(!success) fprintf(stderr, "Failed to write image %s\n", buffer);
}

void l1_normalize(image_t* im)
{
    float sum = 0;
    for(int i = 0; i < im->w*im->h*im->c; ++i) sum += im->data[i];
    for(int i = 0; i < im->w*im->h*im->c; ++i) im->data[i] /= sum;
}

void l2_normalize(image_t* im)
{
    float sum = 0;
    for(int i = 0; i < im->w*im->h*im->c; ++i) sum += im->data[i]*im->data[i];
    sum = sqrtf(sum);
    for(int i = 0; i < im->w*im->h*im->c; ++i) im->data[i] /= sum;
}

void convolve_image(const image_t& in, const image_t& kernel, image_t* out, bool preserve)
{
    assert(in.c == kernel.c || kernel.c == 1);
    bool single_channel = kernel.c == 1;
    *out = make_image(in.w, in.h, preserve ? in.c : 1);
    for(int k = 0; k < in.c; ++k) {
        int kernel_channel = single_channel ? 0 : k, out_channel = preserve ? k : 0;
        for(int j = 0; j < in.h; ++j) {
            for(int i = 0; i < in.w; ++i) {
                for(int dy = 0; dy < kernel.h; ++dy) {
                    for(int dx = 0; dx < kernel.w; ++dx) {
                        float weight = get_pixel(kernel, dx, dy, kernel_channel);
                        float val = get_pixel_extend(in, i-kernel.w/2+dx, j-kernel.h/2+dy, k);
                        add_pixel(out, i, j, out_channel, val*weight);
                    }
                }
            }
        }
    }
}
