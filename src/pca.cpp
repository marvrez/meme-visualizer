#include "pca.h"

void principal_component_sort(principal_components_t* principal_components)
{
    matrix_t* eigen_vecs = &principal_components->eigen_vecs;
    std::vector<float>& eigen_vals = principal_components->eigen_vals;
    int k, n = eigen_vecs->rows;

    for (int i = 0; i < n - 1; ++i) {
        float p = eigen_vals[k = i];

        for (int j = i + 1; j < n; ++j) {
            if (eigen_vals[j] >= p) 
                p = eigen_vals[k = j];
        }

        if (k != i) {
            eigen_vals[k] = eigen_vals[i];
            eigen_vals[i] = p;

            for (int j = 0; j < n; ++j) {
                p = eigen_vecs->vals[j][i];
                eigen_vecs->vals[j][i] = eigen_vecs->vals[j][k];
                eigen_vecs->vals[j][k] = p;
            }
        }
    }
}

principal_components_t pca(const matrix_t& data)
{
    matrix_t cov_mat = covariance_matrix(data);

    principal_components_t pc; //stores the principal components
    pc.eigen_vecs = make_matrix(cov_mat.rows, cov_mat.cols);
    pc.eigen_vals = std::vector<float>(cov_mat.rows, 0);

    jacobi_eigenvalue(&cov_mat, pc.eigen_vals, &pc.eigen_vecs);
    principal_component_sort(&pc);

    return pc;
}
