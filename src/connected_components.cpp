#include "connected_components.h"

#include <queue>

void connected_components_bfs(const image_t& binary, const std::vector<int>& points, std::vector<int>* label) 
{
    *label = std::vector<int>(binary.w*binary.h, -1);
    int count = 0;
    for (int i = 0; i < points.size(); ++i) {
        int root = points[i];
        if ((*label)[root] == -1) {
            std::queue<int> queue;
            queue.push(root);
            while (!queue.empty()) {
                // extract from queue and grow component
                int p = queue.front(); queue.pop();
                if((*label)[p] != -1) continue;

                (*label)[p] = count;

                int neighbors[] = { p+binary.w, p-binary.w, p+1, p-1, p+1+binary.w, p-1+binary.w, p+1-binary.w, p-1-binary.w };
                for (int q : neighbors) {
                    if (binary.data[q] > 0.f && (*label)[q] == -1) {
                        queue.push(q);
                    }
                }
            }
            count++;
        }
    }
}

std::vector<int> connected_components(const image_t& m, cc_options_t opt, std::vector<int>* points)
{
    points->clear();
    image_t binary = make_image_grayscale(m.w, m.h);
    std::vector<int> label(m.w*m.h,-1);

    for (int y = 1; y < m.h - 1; ++y) {
        for (int x = 1; x < m.w - 1; ++x) {
            float r = get_pixel(m,x,y,0), g = get_pixel(m,x,y,1), b = get_pixel(m,x,y,2);
            float norm = r + g + b;
            if (norm > 0.0f) {
                r /= norm, g /= norm, b /= norm;

                bool is_red   = r > opt.r_g*g && r > opt.r_b*b && norm > opt.r_n*3;
                bool is_green = g > opt.g_r*r && g > opt.g_b*b && norm > opt.g_n*3;
                bool is_blue  = b > opt.b_r*r && b > opt.b_g*g && norm > opt.b_n*3;

                set_pixel(&binary, x, y, 0, 0.f);
                if (is_red || is_green /*|| is_blue*/) {
                    points->push_back(y*m.w+x);
                    set_pixel(&binary, x, y, 0, 1.f);
                }
            }
        }
    }
    save_image_png(binary, "binary");
    connected_components_bfs(binary, *points, &label);

    return label;
}

#if 0 
image_t draw_connected_components(const image_t& m, const image_t& points, const std::vector<int>& label)
{
    image_t out = copy_image(m);
    for (int i = 0; i < points.size(); i++) {
        int p = points.data[i];
        int l = label[p];
        vdb_color c = vdbPalette(l);
        set_pixel(&out, 0, c.r);
        set_pixel(&out, 1, c.g);
        set_pixel(&out, 2, c.b);
        I_groups[3*p+0] = c.r;
        I_groups[3*p+1] = c.g;
        I_groups[3*p+2] = c.b;
    }
    return out;
}
#endif
