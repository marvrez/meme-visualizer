#include "data_gen.h"

#include "utilities.h"

#include <iostream>

std::vector<matrix_t> generate_clusters(const matrix_t& centroids, const int max_num_points, const float sigma) 
{
    if (centroids.vals.empty()) {
        std::cout << "[ERROR] No centroids have been imported. Aborting operation.";
        return {};
    }

    std::vector<matrix_t> clusters;
    for (int i = 0; i < centroids.vals.size(); ++i) {
        matrix_t cluster;
        const auto& centroid = centroids.vals[i];
        int dim = centroid.size();
        for(int num_points = 0; num_points < rng0.Rand(max_num_points) + 50; ++num_points) {
            std::vector<float> temp_point(dim);
            for (int d = 0; d < dim; ++d) {
                temp_point[d] = normal_dist(centroid[d], sigma);
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
    float std_x    = sqrtf(sigma.vals[0][0]), covar_xy = sigma.vals[0][1];
    float covar_yx = sigma.vals[1][0],        std_y    = sqrtf(sigma.vals[1][1]);

    const int rows = 334;
    matrix_t x = create_random_normal_matrix(rows, 1, 0, 0.25);
    for(int i = 0; i < x.rows; ++i) {
        float x_new = randn(covar_xy * x.vals[i][0], std_x);
        float y_new = randn(covar_yx * x.vals[i][0], std_y);
        x.vals[i] = { x_new, y_new };
    }
    x.cols = 2;

    return x;
}

matrix_t generate_linear_data_2d(int num_points, float beta, float mu, float sigma)
{
    matrix_t m = make_matrix(num_points, 2);
    std::vector<float> x = linspace(-1.f, 1.f, num_points);

    matrix_t noise = create_random_normal_matrix(num_points, 1, mu, sigma);
    for(int i = 0; i < num_points; ++i) {
        float y = beta * x[i] + noise.vals[i][0];
        m.vals[i] = { x[i], y };
    }

    return m;
}
