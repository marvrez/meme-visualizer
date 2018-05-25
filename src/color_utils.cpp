#include "color_utils.h"

color_t hsv_to_rgb(float h, float s, float v)
{
    float r, g, b, a = 1.0f;
    float f, p, q, t;
    if (s == 0) {
        r = g = b = v;
    } 
    else {
        int index = floor(h * 6);
        f = h * 6 - index;
        p = v*(1-s);
        q = v*(1-s*f);
        t = v*(1-s*(1-f));
        switch (index)
        {
            case 0:
                r = v; g = t; b = p;
                break;
            case 1:
                r = q; g = v; b = p;
                break;
            case 2:
                r = p; g = v; b = t;
                break;
            case 3:
                r = p; g = q; b = v;
                break;
            case 4:
                r = t; g = p; b = v;
                break;
            default:
                r = v; g = p; b = q;
                break;
        }
    }
    return {r, g, b, a};
}

std::vector<color_t> get_colors(const int n) 
{
    std::vector<color_t> colors;
    const float golden_ratio_conjugate = 0.618033988749895f;
    const float s = 0.7f, v = 0.99f;
    for (int i = 0; i < n; ++i) {
        const float h = std::fmod(rng0.getFloat() + golden_ratio_conjugate,
                                  1.0f);
        colors.push_back(hsv_to_rgb(h, s, v));
    }
    return colors;
}
