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

matrix_t generate_covariance_data(const matrix_t& sigma)
{
    matrix_t result = {};
    return result;
}
