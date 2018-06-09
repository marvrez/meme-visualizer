#ifndef PCA_H
#define PCA_H

#include "matrix.h"

#include <vector>

typedef struct {
    matrix_t eigen_vecs;
    std::vector<float> eigen_vals;
} principal_components_t;

void principal_component_sort(principal_components_t* principal_components);
principal_components_t pca(const matrix_t& data);

#endif
