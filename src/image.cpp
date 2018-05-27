#include "image.h"

image_t make_image(int w, int h, int c)
{
    image_t out = {w, h, c, std::vector<float>(w*h*c, 0.f)};
    return out;
}

void clear_image(image_t* m)
{
    m->data = std::vector<float>(m->w*m->h*m->c, 0);
}

void set_pixel(image_t* m, int x, int y, int c, float val)
{
    if (x < 0 || y < 0 || c < 0 || x >= m->w || y >= m->h || c >= m->c) return;
    m->data[(c * m->h * m->w) + (y * m->w) + x] = val;
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
