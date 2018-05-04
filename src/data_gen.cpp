#include "data_gen.h"

#include <iostream>

std::vector<matrix_t> generate_clusters(const matrix_t& centroids, const int max_num_points, const float sigma) 
{
    if (centroids.vals.empty()) 
    {
        std::cout << "[ERROR] No centroids have been imported. Aborting operation.";
        return {};
    }

    std::vector<matrix_t> clusters;
    for (auto centroid_iter = centroids.vals.begin(); centroid_iter != centroids.vals.end(); ++centroid_iter) 
    {
        matrix_t cluster;
        for(int i = 0; i < rng0.Rand(max_num_points) + 50; ++i) 
        {
            std::vector<float> temp_point;
            for (auto dimension_iter = centroid_iter->begin(); dimension_iter != centroid_iter->end(); ++dimension_iter) {
                temp_point.push_back(normal_dist(*dimension_iter, sigma));
            }
            cluster.vals.push_back(temp_point);
        }
        cluster.rows = cluster.vals.size(), cluster.cols = cluster.vals[0].size();
        clusters.push_back(cluster);
    }
    return clusters;
}

// currently assumes 2x2 covariance matrix
matrix_t generate_covariance_data(const matrix_t& sigma)
{
    float var_x    = sigma.vals[0][0], covar_xy = sigma.vals[0][1];
    float covar_yx = sigma.vals[1][0], var_y    = sigma.vals[1][1];

    const int rows = 334;
    matrix_t x = create_random_normal_matrix(rows, 1, 0, 0.25);
    for(int i = 0; i < x.rows; ++i) 
    {
        float x_new = randn(covar_xy * x.vals[i][0], var_x);
        float y_new = randn(covar_yx * x.vals[i][0], var_y);
        x.vals[i] = {x_new, y_new};
    }
    x.cols = 2;

    return x;
}
