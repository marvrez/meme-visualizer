#ifndef DATA_GEN_H
#define DATA_GEN_H

#include "matrix.h"

#include <vector>
#include <random>


static inline float normal_dist(float mu, float sigma) 
{
    static std::mt19937 gen(std::random_device{}());
    std::normal_distribution<float> distr(mu, sigma);
    return distr(gen);
}

// Primary cluster generator - aborts if no centroids have been imported.
// generates clusters with deviations around the given centroids
std::vector<matrix_t> generate_clusters(const matrix_t& centroids, const int max_num_points, const float sigma);

matrix_t generate_covariance_data(const matrix_t& sigma);

#endif
