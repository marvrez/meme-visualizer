#ifndef CONNECTED_COMPONENTS_H
#define CONNECTED_COMPONENTS_H

#include "image.h"

#include <vector>

typedef struct {
    float r_g, r_b, r_n;
    float g_r, g_b, g_n;
    float b_r, b_g, b_n;
} cc_options_t;

void connected_components_bfs(const image_t& binary, const std::vector<std::pair<int,int> >& points, std::vector<int>* label);
std::vector<int> connected_components(const image_t& m, cc_options_t opt, std::vector<std::pair<int,int> >* points);

#endif
